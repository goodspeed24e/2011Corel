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

#include "common.h"
#include "fast.h"
#include "parm.h"
#include "data.h"
#include "prototyp.h"

static int sf_levdur (real[], real LPC_MEM []);
static int levdur (real[], real LPC_MEM [], int);
static void hybwin(int lpsize, int framesize, int nrsize,
	    real old_input[], real new_input[], real output[],
	    real WIN_MEM window[], real rec[],  real decay);
static void bw_expand2(real input[],
		       real COEFF_MEM z_out[], real COEFF_MEM p_out[],
		       int order, real z_vec[], real p_vec[]);
static void bw_expand1(real input[], real COEFF_MEM p_out[],
		int order, real p_vec[]);

/********************************** Adapter for Perceptual Weighting Filter */

static real 
    pwf_z_vec[LPCW+1],		/* Arrays for band widening: zeros and*/
    pwf_p_vec[LPCW+1],		/* poles */
    pwf_old_input[LPCW+NFRSZ+NONRW],
    pwf_rec[LPCW+1];			/* Recursive Part */
    

void
pwf_adapter (real input[],
	     real COEFF_MEM z_out[], /* zero coefficients */
	     real COEFF_MEM p_out[]) /* pole coefficients */
{
    static real
	acorr[LPCW+1],		/* autocorrelation coefficients */
	lpcoeff[LPCW+1];
    static real LPC_MEM
	temp[LPCW+1];

    hybwin(LPCW,	/* lpsize */
	   NFRSZ,	/* framesize */
	   NONRW,	/* nrsize -- nonrecursive size */
	   pwf_old_input,
	   input,
	   acorr,
	   hw_percw,
	   pwf_rec,
	   0.5);
    if (levdur(acorr, temp, LPCW))
	RCOPY(temp, lpcoeff, LPCW+1);
    bw_expand2 (lpcoeff, z_out, p_out, LPCW,
		pwf_z_vec, pwf_p_vec);
}

 void
init_pwf_adapter (real COEFF_MEM z_co[], real COEFF_MEM p_co[])
{
    real zv = 1.0, pv = 1.0;
    int i;
    for (i=0; i<=LPCW; i++)
    {
	pwf_z_vec[i] = zv;
	pwf_p_vec[i] = pv;
	zv *= WZCF;
	pv *= WPCF;
	z_co[i] = 0.0;
	p_co[i] = 0.0;
    }
    p_co[0] = 1.0;
    z_co[0] = 1.0;
    ZARR(pwf_old_input);
    ZARR(pwf_rec);
}

/*************************************** Backward Synthesis Filter Adapter */

static real facv[LPC+1];

static real
    bsf_old_input[LPC+NFRSZ+NONR],
    bsf_rec[LPC+1]; 


void
bsf_adapter (real input[], real COEFF_MEM p_out[])
{
    static real
	old_input[LPC + NFRSZ + NONR],
	acorr[LPC+1],		/* autocorrelation coefficients */
	lpcoeff[LPC+1];
    static real LPC_MEM temp[LPC+1];
    hybwin(LPC,		/* lpsize */
	   NFRSZ,	/* framesize */
	   NONR,	/* nrsize -- nonrecursive size */
	   old_input,
	   input,
	   acorr,
	   hw_synth,
	   bsf_rec,
	   0.75);
    if (sf_levdur(acorr, temp))
	RCOPY(temp, lpcoeff, LPC+1);
    bw_expand1(lpcoeff, p_out, LPC, facv);
}

/******************************************************* Gain Adapter **/

static real gain_p_vec[LPCLG+1];  /* Array for band widening */

static real g_rec[LPCLG+1];	/* Recursive part for Hybrid Window */
static real g_old_input[LPCLG + NUPDATE + NONRLG];

/*** recompute lpc_coeff **/


void gain_adapter (real log_gain[], real COEFF_MEM coeff[])
{
    static real
	acorr[LPCLG+1],		/* autocorrelation coefficients */
	lpcoeff[LPCLG+1];
    
    static real LPC_MEM temp[LPCLG+1];

    hybwin(LPCLG,	/* lpsize */
	   NUPDATE,	/* framesize */
	   NONRLG,	/* nrsize -- nonrecursive size */
	   g_old_input,
	   log_gain,
	   acorr,
	   hw_gain,
	   g_rec,
	   0.75);
    if (levdur(acorr, temp, LPCLG)) {
	int i;
	for(i=1; i<=LPCLG; i++)
	    lpcoeff[i] = temp[i];
    }
    bw_expand1 (lpcoeff, coeff, LPCLG,
		gain_p_vec);
}
/******************************************** Initializations **/

 void
init_bsf_adapter(real COEFF_MEM co[])
{
    int i;
    real v = 1.0;

    for(i=0; i<=LPC; i++) {
	facv[i] = v;
	v *= FAC;
	co[i] = 0;
    }
    co[0] = 1.0;
    ZARR(bsf_old_input);
    ZARR(bsf_rec);
    
}


void init_gain_adapter (real COEFF_MEM coeff[])
{
    real pv = 1.0;
    int i;
    gain_p_vec[0] = 1.0;
    coeff[0] = 1;
    for (i=1; i<=LPCLG; i++)
    {
	pv *= FACGP;
	gain_p_vec[i] = pv;
	coeff[i] = 0;
    }
    for(i=0; i<LPCLG+NUPDATE+NONRLG; i++)
	g_old_input[i] = 0;
    ZARR(g_rec);
    ZARR(g_old_input);
}

/******************************************** Hybrid Window Module **/

/*
  Hybrid Window 

  LPSIZE	- size of OUTPUT (autocorrelation vector)
  FRAMESIZE	- size of NEW_INPUT
  NRSIZE	- size of non-recursive part.
  OLD_INPUT	- buffer for holding old input (size LPSIZE+FRAMESIZE+NRSIZE)
  NEW_INPUT	- new input, or frame (size FRAMESIZE)
  OUTPUT	- autocorrelation vector (size LPSIZE)
  WINDOW	- window coefficients (size LPSIZE+FRAMESIZE+NRSIZE)
  REC	- 	- recursive part (size LPSIZE)
  DECAY	- 	- scaling for the old recursive part.
 */


static void
hybwin(int lpsize, int framesize, int nrsize, real old_input[], 
       real new_input[], real output[],real WIN_MEM window[],
		    real rec[],		/* Recursive Part */
		    real decay)
{
    int N1 = lpsize + framesize; /* M+L */
    int N2 = lpsize + nrsize;	/* M+N */
    int N3 = lpsize + framesize + nrsize;
    int i;
    real ws[N3];
    real tmp1[lpsize+1], tmp2[lpsize+1];


    /* shift in INPUT into OLD_INPUT and window it*/
    
    for(i=0; i<N2; i++)
	old_input[i] = old_input[i+framesize];
    for(i=0; i<framesize; i++)
	old_input[N2+i] = new_input[i];

    VPROD(old_input,window,ws,N3);
    
    AUTOCORR(ws, tmp1, lpsize, lpsize, N1);
    
    for(i=0; i<=lpsize; i++)
	rec[i] = decay * rec[i] + tmp1[i];
    
    AUTOCORR(ws, tmp2, lpsize,  N1, N3);
    
    for(i=0; i<=lpsize; i++)
	output[i] = rec[i] + tmp2[i];

    output[0] *= WNCF;
}


/********************************************* Levinson-Durbin Routines */


/* Levinson-Durbin algorithm */
/* return 1 if ok, otherwise 0 */


static int levdur(real acorr[], real LPC_MEM coeff[], int order)
{
    real K, E;
    int m,j, halfm;

    E = acorr[0];
    if (E<=0) 
	return 0;
    coeff[0] = 1.0;

    for(m=1; m<=order; m++) {
	K =  -acorr[m];
	if (m>1)
	{
	    real c1=coeff[1], a1=acorr[m-1];
	    for(j=2; j<=m-1; j++) {
		K -= c1 * a1;
		c1 = coeff[j];
		a1 = acorr[m-j];
	    }
	    K -= a1*c1;
	}
	K = K/E;
	coeff[m] = K;
	halfm = m>>1;

	/** this is pipilened version of parallel assignment:  **/
	/*  coeff[j]   =     coeff[j] + K * coeff[m-j] */
	/*  coeff[m-j] = K * coeff[j] +     coeff[m-j] */

      {
	  if (halfm>0) {
	      real x,y,t1,t2,t3,t4;
	      real LPC_MEM *p = coeff+1;
	      real LPC_MEM *q = coeff+m-1;
	      x=*p;
	      y=*q;
	      t1 = K * x;
	      t2 = K * y;
	      for(j=1; j<halfm; j++) {
		  t3 = t1 + y;
		  t4 = t2 + x;
		  x=*(p+1);
		  y=*(q-1);
		  *p++ = t4;
		  *q-- = t3;
		  t1 = K * x;
		  t2 = K * y;
	      }
	      t3 = t1 + y;
	      t4 = t2 + x;
	      *p = t4;
	      *q = t3;
	  }
      }
	E = (1 - K * K) * E;
	if (E<0)
	    return 0;
    }
    return 1;
}

/*
  Levinson-Durbin algorithm  for Synthesis Filter. Its main 
  difference from the above is the fact that it saves 10-th
  order coefficients for Postfilter, plus some speedup since this 
  is one of the longest routines in the algorithm.
  */ 

real PSF_LPC_MEM  a10[11];
real PSF_LPC_MEM  k10;

static int sf_levdur(real acorr[], real LPC_MEM coeff[])
{
    real E;
    register real K REG(r9), c1 REG(r0), tmp REG(r12);
    int m, j, halfm;

    E = acorr[0];
    if (E<=0) 
	return 0;
    coeff[0] = 1.0;
    for(m=1; m<=LPC; m++) {
	K =  -acorr[m];
	if (m>1)
	{
	    real a1 REG(r4)=acorr[m-1];
	    c1=coeff[1];
	    tmp = c1*a1;
	    if (m>2) {
		c1 = coeff[2]; a1 = acorr[m-2];
		for(j=3; j<=m-1; j++) {
		    K -= tmp;
		    tmp = c1 * a1;
		    c1 = coeff[j];
		    a1 = acorr[m-j];
		}
		K -= tmp;
		tmp = c1*a1;
	    }
	    K -= tmp;
	}
	K = K/E;
	coeff[m] = K;
	halfm = m>>1;

	/** this is pipilened version of parallel assignment:  **/
	/*  coeff[j]   =     coeff[j] + K * coeff[m-j] */
	/*  coeff[m-j] = K * coeff[j] +     coeff[m-j] */

      {
	  if (halfm>=1) {
	      register real
		  x  REG(r1),   y  REG(r5), t1 REG(r10),
		  t2 REG(r14), t3 REG(r2), t4 REG(r6);
	      register real LPC_MEM *p  REG(i10);
	      register real LPC_MEM *pp REG(i11);
	      register real LPC_MEM *q  REG(i12);
	      register real LPC_MEM *qq REG(i13);

	      p = coeff+1;   pp = p;
	      q = coeff+m-1; qq = q;
	      x=*p++;
	      y=*q--;
	      t1 = K * x;
	      t2 = K * y;
	      for(j=2; j<=halfm; j++) {
		  t4 = t2 + x;	 x=*p++;
		  t3 = t1 + y;   y=*q--;      
		  t1 = K * x;	*pp++ = t4;
		  t2 = K * y;	*qq-- = t3;
	      }
	      t3 = t1 + y;
	      t4 = t2 + x;
	      *pp = t4;
	      *qq = t3;
	  }
      }
	if (m==10) {
	    int jj;
	    for(jj=0; jj<=10; jj++)
		a10[jj] = coeff[jj];
	    k10 = K;
	}
	E = (1 - K * K) * E;
	if (E<0)
	    return 0;
    }
    return 1;
}

/******************************************** Band Width Expanders **/

/* Don't have to worry about i=0 -- z_vec[0] and p_vec[0] should stay 1.0. */

static void
bw_expand2(real input[], real COEFF_MEM z_out[], real COEFF_MEM p_out[],
		       int order, real z_vec[], real p_vec[])
{
	int i;
	for(i=1; i<=order; i++)
	    z_out[i] = z_vec[i] * input[i];
	for(i=1; i<=order; i++)
	    p_out[i] = p_vec[i]*input[i];
}

/* Poles only */

static void
bw_expand1(real input[], COEFF_MEM real p_out[], int order, real p_vec[])
{
	int i;
	for(i=1; i<=order; i++)
	    p_out[i] = p_vec[i]*input[i];
}
