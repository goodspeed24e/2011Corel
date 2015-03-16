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

/* Multitasking -- works on a DSP target only !! */

#include <stdio.h>
#include <signal.h>
#include "common.h"
#include "parm.h"
#include "prototyp.h"

#ifdef __ADSP21000__
#include "21kflags.h"
#endif

extern void init_output();
void init_decoder();
void decoder();
void decode_vector(int);
void adapt_decoder();

int postfiltering_p = 1;

static int d_vec_start;	/* Index of the start of the decoded speech vector */
static int d_vec_end;	/* Index of the end   of the decoded speech vector */
static int w_vec_start;	/* Index of the start of vector being written */
static int w_vec_end;	/* Index of the end   of vector being written */

int VOLATILE decoder_done = 0;

#ifdef MAIN

extern char *ofile_name;
extern char *xfile_name;

void main (int argc, char *argv[])
{
    static void usage(char*);
    
    if (argc != 3)
	usage(argv[0]);
#ifdef DEBUG
  {
      int fnum;
      for (fnum=0; fnum<argc; fnum++)
	  (void) sp_debug_switch(argv[fnum]);
  }
#endif

    xfile_name = argv[1];
    ofile_name = argv[2];
    init_decoder();
    ffase = -4;
    decoder();
}

static void usage(char *name)
{
    fprintf(stderr, "Usage: %s  <index-file> <audio-file>\n", name);
    exit(0);
}
#endif


#ifdef __ADSP21000__
void decoder()
{
    init_decoder();

    /* Adapter runs in the background while the real coder is invoked */
    /* as software (user) interrupt routine */
        
    decoder_done = 0;
    while(!decoder_done)
	adapt_decoder();
}

#else

real thequeue[QSIZE];
real *vector_end;

void decoder()
{
    int i;
    for(i=0; i<QSIZE; i++) thequeue[i] = 0; 
    init_decoder();
    for(w_vec_start=0; ! decoder_done; w_vec_start+=IDIM) {
	if (w_vec_start >= QSIZE)
	    w_vec_start = 0;
	write_sound_buffer(IDIM, thequeue + w_vec_start);
	w_vec_end = w_vec_start + IDIM;
	vector_end = thequeue + w_vec_end;
	decode_vector(0);
	adapt_decoder();
    }
}
#endif

void init_decoder()
{
    init_bsf_adapter(sf_coeff);
    sf_coeff_obsolete_p = 0;
    init_gain_adapter(gp_coeff);
    gp_coeff_obsolete_p = 0;
    init_output();
    vector_end=thequeue;
#ifdef __ADSP21000__
    mode2.b.cafrz = 0;
    interrupt(SIG_USR0, decode_vector);
#endif
}

void decode_vector(int ignore)
{
    int ix;	/* Computed Codebook Index */
    int lgx;	/* Log Gains INdex */
    static real zero_response[IDIM], cb_vec[IDIM];
    static real pf_speech[IDIM];

    static real	gain =1.0;
    w_vec_end = vector_end - thequeue;
    d_vec_start = w_vec_end + IDIM;
    if (d_vec_start >= QSIZE)
	d_vec_start -= QSIZE;
    ix = get_index();
    if (ix < 0)
	decoder_done = 1;

    UPDATE(sf_coeff);
    zresp(zero_response);
    				SPDEBUG(20,zero_response, IDIM);
    cb_excitation(ix, cb_vec);
    				SPDEBUG(21,cb_vec, IDIM);
    sig_scale(gain, cb_vec, qspeech+d_vec_start);
    				SPDEBUG(22,qspeech+d_vec_start, IDIM);
    UPDATE(gp_coeff);
    lgx = d_vec_start/IDIM;
    gain = predict_gain(qspeech+d_vec_start, log_gains+lgx);
    				SPDEBUG(23,&gain, 1);
    mem_update(qspeech+d_vec_start, synspeech+d_vec_start);
    				SPDEBUG(24,synspeech+d_vec_start, IDIM);
    if (postfiltering_p) {
	postfilter(synspeech+d_vec_start, pf_speech);
				SPDEBUG(25, pf_speech, IDIM);
	RCOPY(pf_speech, thequeue+d_vec_start, IDIM);
    }
    else {
	RCOPY(synspeech+d_vec_start, thequeue+d_vec_start, IDIM);
    }

    d_vec_end = d_vec_start + IDIM;
    if (d_vec_end >= QSIZE)
	d_vec_end -= QSIZE;
    NEXT_FFASE;
}

void adapt_decoder()
{
    real
	qs    [NUPDATE*IDIM],
	synth [NUPDATE*IDIM],
	lg    [NUPDATE];
    int
	gx; /* gain index */
    
    
    FFASE(1)
    {
	CIRCOPY(synth, synspeech, d_vec_end, NUPDATE*IDIM, QSIZE);
	bsf_adapter (synth, sf_coeff_next);
	sf_coeff_obsolete_p = 1;
    }
	
    FFASE(3)
    {
	gx = d_vec_end / IDIM;
	CIRCOPY(lg, log_gains, gx, NUPDATE, QSIZE/IDIM);
	gain_adapter(lg, gp_coeff_next);
	gp_coeff_obsolete_p = 1;
    }
    
    if (postfiltering_p) {
	FFASE(3) 
	{
	    CIRCOPY(qs, qspeech, d_vec_end, NUPDATE*IDIM, QSIZE);
	    psf_adapter(qspeech);
	}
    }
}
