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

/* Postfilter of LD-CELP Decoder */

#include "common.h"
#include "prototyp.h"
#include "parm.h"
#include "data.h"

#ifdef __ADSP21000__
#include <math.h>
#define ABS(X) fabsf(X)
#else
#define ABS(X) ((X)<0.0?(-X):(X))
#endif

/*  Parameters from the adapter: */

#define SPORDER 10

int  pitch_period = 50;

real pitch_tap=0.0;

real pitch_gain=1;

static real
    shzscale[SPORDER+1],	/* Precomputed Scales for IIR coefficients */
    shpscale[SPORDER+1],	/* Precomputed Scales for FIR Coefficients */
    shpcoef[SPORDER+1],	/* Short Term Filter (Poles/IIR) Coefficients */
    shzcoef[SPORDER+1], /* Short Term Filter (Zeros/FIR) Coefficients */
    tiltz;

static void  longterm(real[], real[]);
static void shortterm(real[], real[]);

/* Compute sum of absolute values of vector V */

static real vec_abs(real v[])
{
    int i;
    real r = v[0];
    for(i=1; i<IDIM; i++)
	r+=ABS(v[i]);
    return r;
}

void
postfilter(real input[], real output[])
{
    int i;
    static
    real
	temp[IDIM], 	/* Output of long term filter*/
	temp2[IDIM], 	/* Input of short term filter*/
	new_gain,	/* Gain of filtered output */
	input_gain, 	/* Gain of input */
	scale;		/* Scaling factor for gain preservation */

	static real scalefil=1.0;	/* Smoother version of scale */

    longterm(input, temp);
    shortterm(temp, temp2);

    /* Computed scale for gain preservation */

    new_gain = vec_abs(temp2);
    if (new_gain > 1.0)
    {
	input_gain = vec_abs(input);
	scale = input_gain/new_gain;
    }
    else 
	scale = 1.0;
    
    /* Smooth out scale, then scale the output */

    for(i=0; i<IDIM; i++) {
	scalefil = AGCFAC * scalefil + (1.0 - AGCFAC)*scale;
	output[i] = scalefil * temp2[i];
    }
}

static void
longterm(real input[], real output[])
{
    int i;
    real out;
    static real lmemory[KPMAX];

    /* Add weighted pitch_period-delayed signal */

    for(i=0; i<IDIM; i++) {
	out = input[i] + pitch_tap * lmemory[KPMAX+i-pitch_period];
	output[i] = pitch_gain*out;
    }
    
    /* Shift-in input to lmemory */

    for (i=0; i<KPMAX-IDIM; i++)
	lmemory[i] = lmemory[i+IDIM];
    for(i=0; i<IDIM; i++)
	lmemory[KPMAX-IDIM+i] = input[i];
}

/*
  Again, memories (shpmem, shzmem) are in reverse order, 
  i.e. [0] is the oldest.
 */

static void
shortterm(real input[], real output[])
{
    int k,j;

    static real shpmem[SPORDER], shzmem[SPORDER];
    for(k=0; k<IDIM; k++) {

	/* FIR Part */

	real in = input[k], out;
	out = in;
	for(j=SPORDER-1; j>=1; j--) {
	    out += shzmem[j]*shzcoef[j+1];
	    shzmem[j] = shzmem[j-1];
	}
	out += shzmem[0] * shzcoef[1];
	shzmem[0] = in;

	/* IIR Part */
	
	for(j=SPORDER-1; j>=1; j--) {
	    out -= shpmem[j]*shpcoef[j+1];
	    shpmem[j] = shpmem[j-1];
	}
	out -= shpmem[0] * shpcoef[1];
	shpmem[0] = out;
	output[k] = out+tiltz*shzmem[1];
    }
}


	/*********************************/
	/***** Postfilter Adapter ********/
	/*********************************/

#define DECIM 4
#define PMSIZE  (NPWSZ+KPMAX)	/* Postfilter Memory SIZE */
#define PDMSIZE (PMSIZE/DECIM)  /* Post. Decim. Memory SIZE */
#define DPERMAX (KPMAX/DECIM)	/* Max. Decimated Period */
#define DPERMIN (KPMIN/DECIM)	/* Min. Decimated Period */

/* 
  Using macro will make it easy to reverse the order of fil_mem elements.
  Current macro realizes reverse memory.
  */

static real fil_mem[PMSIZE];	/* Post-Filter Memory */
#define FIL_MEM(I) fil_mem[PMSIZE-1-(I)]

static int extract_pitch(real[]);

void
psf_adapter (real frame[])
{
    real residue[NFRSZ];

    /* Inverse Filter */

  {
      int i,j,k;
      static real mem1[SPORDER+NFRSZ];

      /** Shift in frame into mem1 **/
      for(i=NFRSZ; i<SPORDER+NFRSZ; i++)
	  mem1[i-NFRSZ] = mem1[i];
      for(i=0; i<NFRSZ; i++)
	  mem1[SPORDER+i] = frame[i];
      
#ifdef noPIPELINE
	{
	    for(k=0; k<NFRSZ; k++) {
		real *mem1kp = mem1+k;
		real
		    PSF_LPC_MEM *ap=a10;
		real
		    *mem1p = mem1kp,
		    mem1_1 = *mem1p,
		    a10_1 = *ap,
		    tmp = mem1_1;
		for(j=1; j<=SPORDER; j++) {
		    tmp -= mem1_1 * a10_1;
		    mem1_1 = *mem1p--;
		    a10_1 = *ap++;
		    }
		residue[k] = tmp-mem1_1*a10_1;
		mem1kp++;
	    }
	}
#else
      /* Un-pipelined version -- kept for reference */
      for(k=0; k<NFRSZ; k++) {
	  real tmp = mem1[SPORDER+k];
	  for(j=1; j<=SPORDER; j++)
	      tmp -= mem1[SPORDER+k-j]*a10[j];
	  residue[k] = tmp;
      }
#endif
  }
    				SPDEBUG(36, residue, NFRSZ);
    pitch_period = extract_pitch(residue);

    /** Compute Pitch Tap **/
  {
      int i;
      real corr=0.0, corr_per=0.0;
#ifdef noPIPELINE
    {
	/** Only for REVERSED (!) fil_mem **/
	real
	    *p1 = &FIL_MEM(KPMAX),
	    *p2 = &FIL_MEM(KPMAX-pitch_period),
	    c1 = *p1--, 
	    c2 = *p2--;
	for(i=1; i<NPWSZ; i++) {
	    corr_per += c1 * c2; 
	    corr += c1 * c1;
	    c2 = *p2--;
	    c1= *p1--;
	}
	corr_per += c1 * c2;
	corr     += c1 * c1;
    }
#else
      /* Un-pipelined version -- kept for reference */
      for(i=0; i<NPWSZ; i++) {
	  corr     += FIL_MEM(KPMAX+i) * FIL_MEM(KPMAX+i);
	  corr_per += FIL_MEM(KPMAX+i) * FIL_MEM(KPMAX-pitch_period+i);
      }
#endif
      if REALZEROP(corr)
	  pitch_tap = 0.0;
      else
	  pitch_tap = corr_per/corr;
  }
    
    /** Compute Long Term Coefficients **/
    
  {
      if (pitch_tap > 1)
	  pitch_tap = 1.0;
      if (pitch_tap < PPFTH)
	  pitch_tap = 0.0;
      pitch_tap = PPFZCF * pitch_tap;
      pitch_gain = 1.0/(1.0+pitch_tap);
  }

    /** Compute Short Term Coefficients **/
  {
      int i;
      for(i=1; i<=SPORDER; i++) {
	  shzcoef[i] = shzscale[i]*a10[i];
	  shpcoef[i] = shpscale[i]*a10[i];
      }
      tiltz = TILTF * k10;
  }
}

static int best_period (real buffer[], int buflen,
			int pmin, int pmax)
{
    int i, per, best_per = -1;
    real best_corr = -(BIG);
    for(per = pmin; per<pmax; per++) {
	real corr = 0.0;
#ifdef noPIPELINE
      {
	  /** Pipelining **/

	  real
	      *pb0 = buffer+pmax,
	      *pb1 = pb0-per,
	      b0 = *pb0,
	      b1 = *pb1;
	  for(i=pmax+1; i<buflen; i++) {
	      corr += b0 * b1;
	      b0 = *pb0++;
	      b1 = *pb1++;
	  }
	  corr += b0 * b1;
      }
#else

	for(i=pmax; i<buflen; i++)
	    corr += buffer[i] * buffer[i-per];
#endif
	if (corr > best_corr) {
	    best_corr = corr;
	    best_per  = per;
	}	    
    }
    return best_per;
}

#define DCFRSZ NFRSZ/DECIM /* size of decimated frame */

static int
extract_pitch(real frame[])
{
    int
	i, j, k,
	best_per=KPMAX,		 /* Best Period (undecimated) */
	best_dper = KPMAX/DECIM, /* Best Decimated Period */
	best_old_per=KPMAX,	 /* Best Old Period */
	permin,			 /* Limits for search of best period */
	permax;
    real
	best_corr=-(BIG), best_old_corr=-(BIG), tap0=0.0, tap1=0.0;
    static int old_per = (KPMIN+KPMAX)>>1;
    static real
	fil_decim_mem[PDMSIZE],
	fil_out_mem[NFRSZ+DECIM];

#define FIL_DECIM_MEM(I) fil_decim_mem[I]
#define FIL_OUT_MEM(I)   fil_out_mem[I]
    
    /** Shift in the frame into fil_mem **/

    for(i=NFRSZ; i<PMSIZE; i++)
	FIL_MEM(i-NFRSZ) = FIL_MEM(i);
    for(i=0; i<NFRSZ; i++)
	FIL_MEM(PMSIZE-NFRSZ+i) = frame[i];

    /** Shift decimated filtered output */

    for(i=DCFRSZ; i<PDMSIZE; i++)
	FIL_DECIM_MEM(i-DCFRSZ) = FIL_DECIM_MEM(i);
  
    /* Filter and  decimate  input */

  {
      int decim_phase = 0, dk;
      for (k = 0, dk=0; k<NFRSZ; k++)
      {
	  real tmp;
	  tmp = frame[k] - A1 * FIL_OUT_MEM(2)
	                 - A2 * FIL_OUT_MEM(1)
	                 - A3 * FIL_OUT_MEM(0);
	  decim_phase++;
	  if (decim_phase == 4) {
	      FIL_DECIM_MEM(PDMSIZE-DCFRSZ+dk) = 
		  B0 * tmp
		+ B1 * FIL_OUT_MEM(2)
		+ B2 * FIL_OUT_MEM(1)
		+ B3 * FIL_OUT_MEM(0);
	      decim_phase = 0;
	      dk++;
	  }
	  FIL_OUT_MEM(0) = FIL_OUT_MEM(1);
	  FIL_OUT_MEM(1) = FIL_OUT_MEM(2);
	  FIL_OUT_MEM(2) = tmp;
      }
      SPDEBUG(27, fil_decim_mem+PDMSIZE-DCFRSZ, 5);
  }

    /* Find best Correlation in decimated domain: */

    best_dper = best_period(fil_decim_mem, PDMSIZE, DPERMIN, DPERMAX);

    /* Now fine-tune best correlation on undecimated  domain */
    
    permin = best_dper * DECIM - DECIM + 1;
    permax = best_dper * DECIM + DECIM - 1;
    if (permax > KPMAX)
	permax = KPMAX;
    if (permin < KPMIN)
	permin = KPMIN;
    
  {
      int per;
      best_corr = -(BIG);
      for(per = permin; per<=permax; per++) {
	  real corr = 0.0;
	  for(i=0,j=per;   i<NPWSZ;   i++,j++)
	      corr += FIL_MEM(PMSIZE-i)*FIL_MEM(PMSIZE-j);
	  if (corr > best_corr) {
	      best_corr = corr;
	      best_per = per;
	  }
      }
  }
    
    /** If we are not exceeding old period by too much, we have a real
       period and not a multiple */
    
    permax = old_per + KPDELTA;
    if (best_per <= permax)
	goto done;

    /** Now compute best period around the old period **/
    
    permin = old_per - KPDELTA;
    if (permin<KPMIN) permin = KPMIN;
  {
      int per;
      best_old_corr = -(BIG);
      for(per = permin; per<permax; per++) {
	  real corr = 0.0;
	  for(i=0,j=per;
	      j<PMSIZE;
	      i++,j++)
	      corr += FIL_MEM(i)*FIL_MEM(j);
	  if (corr > best_old_corr) {
	      best_old_corr = corr;
	      best_old_per = per;
	  }
      }
  }
    
    /***** Compute the tap ****/

  {
      real s0=0.0, s1=0.0;
      for(i=KPMAX; i<PMSIZE; i++) {
	  s0 += FIL_MEM(i-best_per)     * FIL_MEM(i-best_per);
	  s1 += FIL_MEM(i-best_old_per) * FIL_MEM(i-best_old_per);
      }
      if (! REALZEROP(s0))
	  tap0 = best_corr/s0;
      if (! REALZEROP(s1))
	  tap1 = best_old_corr/s1;
      tap0 = CLIPP(tap0, 0.0, 1.0);
      tap1 = CLIPP(tap1, 0.0, 1.0);
      if (tap1 > TAPTH * tap0)
	  best_per = best_old_per;
  }
  done:
    old_per = best_per;
    return best_per;
}

void
init_postfilter()
{
    int i;
    shzscale[0] = shpscale[0] = 1.0;
    for (i=1; i<=SPORDER; i++)
    {
	shzscale[i] = SPFZCF * shzscale[i-1];
	shpscale[i] = SPFPCF * shpscale[i-1];
    }
}

#ifdef PITCHTEST

/* Test Pitch Extract -- only on hosts.
   Build a "chirp wave" and dump expected and computed pitch,
   as well as the filtered and decimated signal */

#include <math.h>

void
main ()
{
    real freq0=70, slope=0.1;
    int i, phase;
    real freq, real_freq, frame[NFRSZ];
    
    sp_file_on(27);
    sp_file_on(30);
    sp_file_on(33);
    sp_file_on(34);
    sp_file_on(35);

    for(i=0, phase =0, freq=freq0;
	i<2000;
	i++,phase++,freq+=slope) {
	if (phase == NFRSZ) {
	    int pitch = extract_pitch(frame);
	    real rpitch = (real) pitch;
	    real expected_pitch = 8000/freq;
	    phase= 0;
					    SPDEBUG(33, frame, NFRSZ);
					    SPDEBUG(34, &expected_pitch, 1);
					    SPDEBUG(35, &rpitch, 1);
	}
	real_freq = 2. *M_PI * freq / 8000.;
	frame[phase] = sin(real_freq*i);
    }
}
#endif
