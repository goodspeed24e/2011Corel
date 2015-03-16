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

/********************************* Perceptual Weighting Filter **/

#include "common.h"
#include "parm.h"
#include "fast.h"
#include "prototyp.h"

/* Filter Memory in reverse order, i.e. firmem[0] is the most recent */

#ifdef __ADSP21000__
static real pm firmem[LPCW+IDIM];
static real pm iirmem[LPCW+IDIM];
#else
static real firmem[LPCW+IDIM];
static real iirmem[LPCW+IDIM];
#endif



void pwfilter2(real QMEM input[], real output[])
{
    int k;
    real out;

    RSHIFT(firmem, LPCW, IDIM);
    for(k=0; k<IDIM; k++)
	firmem[k] = input[IDIM-1-k];
    RSHIFT(iirmem, LPCW, IDIM);

    for (k=0; k<IDIM; k++) {
	out = firmem[IDIM-1-k];		/* pwf_z_coeff[0] is always 1.0 */
	out += DOTPROD(firmem+IDIM-k, pwf_z_coeff+1, LPCW);
	out -= DOTPROD(iirmem+IDIM-k, pwf_p_coeff+1, LPCW);
	iirmem[IDIM-1-k] = out;
	output[k] = out;
    }
}

/*  Synthesis and Perceptual Weighting Filter. */

#ifdef __ADSP21000__
#define STATE_MEM pm
#define ZF_MEM    pm
#define ZI_MEM    pm
#else
#define STATE_MEM
#define ZF_MEM   
#define ZI_MEM   
#endif

real STATE_MEM statelpc[LPC+IDIM];
real ZF_MEM    zirwfir[LPCW];
real ZI_MEM    zirwiir[LPCW];

/** Updateable coefficients **/

void sf_zresp(real []);
void pwf_zresp(real [], real[]);

void sf_zresp(real output[])
{
  int k,j;

#if PIPELINE
  for(j=LPC-1; j>=0; j--)
      statelpc[j+IDIM] = statelpc[j];

{
    real STATE_MEM * sjpk = statelpc + LPC+IDIM-1; 
    real out = 0.0;
    for (k=0; k<IDIM; k++) {
      real COEFF_MEM *ajp = sf_coeff+LPC;
      real sj, aj;
      real STATE_MEM *sjp;
      out = 0.0;
      sjp = sjpk;
      sj = *sjp--;
      aj = *ajp--;
      for (j=LPC-2; j>=0; j--) {
	  out -= sj*aj;
	  sj = *sjp--;
	  aj = *ajp--;
      }
      output[k] = out - sj*aj;
      *sjp--= output[k];
      sjpk--;
  }
}
#else
/** This is un-pipelined version of the above. Kept for reference **/
    for(j=LPC-1; j>=0; j--)
      statelpc[j+IDIM] = statelpc[j];
  for (k=0; k<IDIM; k++) {
      real out = 0.0, sj, aj;
      sj = statelpc[LPC+IDIM-k-1];
      aj = sf_coeff[LPC];
      for (j=LPC-2; j>=1; j--) {
	  out -= sj*aj;
	  sj = statelpc[IDIM-k+j];
	  aj = sf_coeff[j+1];
      }
      output[k] = out - sj*aj-statelpc[IDIM-k] * sf_coeff[1];
      statelpc[IDIM-1-k] = output[k];
  }
return;
#endif
}

void
pwf_zresp(real input[], real output[])
{
   int j,k;
   real tmp;

#if PIPELINE
   for (k=0; k<IDIM; k++) {
   	tmp = input[k];
   	for (j=LPCW-1; j>=1; j--) {
   	   input[k] += zirwfir[j] * pwf_z_coeff[j+1];
   	   zirwfir[j] = zirwfir[j-1];
   	}
	input[k] += zirwfir[0] * pwf_z_coeff[1];
   	zirwfir[0] = tmp;
   	for (j=LPCW-1; j>=1; j--) {
   	    input[k] -= zirwiir[j] * pwf_p_coeff[j+1];
   	    zirwiir[j] = zirwiir[j-1];
   	}
   	output[k] = input[k] - zirwiir[0] * pwf_p_coeff[1];
   	zirwiir[0] = output[k];
   }
#else
   /** Un-pipelined version, kept for reference **/
   for (k=0; k<IDIM; k++) {
   	tmp = input[k];
   	for (j=LPCW-1; j>=1; j--) {
   	   input[k] += zirwfir[j] * pwf_z_coeff[j+1];
   	   zirwfir[j] = zirwfir[j-1];
   	}
	input[k] += zirwfir[0] * pwf_z_coeff[1];
   	zirwfir[0] = tmp;
   	for (j=LPCW-1; j>=1; j--) {
   	    input[k] -= zirwiir[j] * pwf_p_coeff[j+1];
   	    zirwiir[j] = zirwiir[j-1];
   	}
   	output[k] = input[k] - zirwiir[0] * pwf_p_coeff[1];
   	zirwiir[0] = output[k];
   }
#endif   
}


void zresp(real output[])
{
    real temp[IDIM];
    sf_zresp(temp);
    pwf_zresp(temp, output);
}


void mem_update (real input[], real output[])
{
    int i,k;
    real temp[IDIM], a0, a1, a2;
    real STATE_MEM *t2 = zirwfir;
    t2[0] = temp[0] = input[0];
    for (k=1; k<IDIM; k++) {
	a0 = input[k];
	a1 = a2 = 0.0;
	for (i=k; i>= 1; i--) {
	    t2[i] = t2[i-1];
	    temp[i] = temp[i-1];
	    a0 -=   sf_coeff[i] * t2[i];
	    a1 += pwf_z_coeff[i] * t2[i];
	    a2 -= pwf_p_coeff[i] * temp[i];
	}
	t2[0] = a0;
	temp[0] = a0+a1+a2;
    }
    
    for (k=0; k<IDIM; k++) {
   	statelpc[k] += t2[k];
   	if (statelpc[k] > Max)
	    statelpc[k] = Max;
        else if (statelpc[k] < Min)
	    statelpc[k] = Min;
        zirwiir[k] += temp[k];
    }
    for (i=0; i<LPCW; i++)
   	zirwfir[i] = statelpc[i];
    for (k=0; k<IDIM; k++)
	output[k] = statelpc[IDIM-1-k];
    return;
}

#include <math.h>

#ifdef __ADSP21000__
# define LOG10(X) log10f(X)
# define EXP10(X) expf(2.302585092994046 * (X))
extern float exp10f(float);
#else
# define LOG10(X) log10(X)
# define EXP10(X) exp10(X)
#endif

/*********************************************** The Gain Predictor */

extern real COEFF_MEM gp_coeff[];
static real gain_input[LPCLG];

static real log_rms(real input[])
{
    real etrms=0.0;
    int k;
    for(k=0; k<IDIM; k++)
	etrms += input[k]*input[k];
    if (etrms<1.0)
	etrms = 1.0;
    etrms /= IDIM;
    etrms = 10.0f * LOG10(etrms);
    return (etrms);
}

static real predict_log_gain(real g)
{
    int i;
    real new_gain = 0;
	    				 SPDEBUG(31, &g, 1);
    for (i=0; i<LPCLG-1; i++) {
	gain_input[i] = gain_input[i+1];
	new_gain -= gp_coeff[LPCLG-i] * gain_input[i];
    }
    gain_input[LPCLG-1] = g;
    new_gain -= gp_coeff[1] * gain_input[LPCLG-1];
	    				SPDEBUG(32, &new_gain, 1);
    return (new_gain);
}

real predict_gain(real input[], real *lgp)
{
    real new_gain, log_gain, tmp;
    *lgp = log_gain = log_rms(input) - GOFF;
    tmp = GOFF + predict_log_gain(log_gain);
    if (tmp <  0.0) tmp =  0.0;
    if (tmp > 60.0) tmp = 60.0;
    new_gain = EXP10(0.05*tmp);
    						CHECK1(new_gain);
    return (new_gain);
}
