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


/* Real multitasking works on a DSP target only, on other platform
   it is simulated */

#include <stdio.h>
#include <signal.h>

#include "common.h"
#include "parm.h"
#include "prototyp.h"

#ifdef __ADSP21000__
#include "21kflags.h"
#endif

extern void init_input();
void init_encoder();
void encoder();
void encode_vector(int);
void adapt_frame();

static int dec_end;	/* Index of the end of the decoded speech */
int VOLATILE encoder_done = 0;

int shifts[4] = {1,3,4,4};
#ifdef __ADSP21000__
#undef FFASE
int ffctr[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
int fff[4]={0,0,0,0};
#define FFASE(N)fff[N-1]++;while(ffase != N)if(ffase>0)ffctr[N-1][ffase-1]++;
#endif

#ifdef MAIN

extern char *ifile_name;
extern char *xfile_name;

void main(int argc, char *argv[])
{
    void encoder();
    static void usage(char*);
    
    if (argc != 3)
	usage(argv[0]);
#ifdef __ADSP21000__
    initx();
#else
    ifile_name = argv[1];
    xfile_name = argv[2];
#endif
    ffase = -4;
    encoder();
}

static void usage(char *name)
{
    fprintf(stderr, "Usage: %s <audio-file> <index-file>\n", name);
    exit(0);
}
#endif

#ifdef __ADSP21000__
void encoder()
{
    init_encoder();

    /* Adapter runs in the background while the real coder is invoked */
    /* as software (user) interrupt routine */
        
    encoder_done = 0;
    while(! encoder_done)
	adapt_frame();
}

#else

real thequeue[QSIZE];
real * vector_end;

void encoder()
{
    int vnum,i;

    init_encoder();
    for(i=0; i<QSIZE; i++) 
	thequeue[i]=0;
    for(vnum=0; 
	read_sound_buffer(IDIM, thequeue + (vnum * IDIM)%QSIZE) > 0;
	vnum++) 
    {
	vector_end = thequeue+(vnum*IDIM)%QSIZE+IDIM;
	encode_vector(0);
	adapt_frame();
    }
}
#endif

void init_encoder()
{
    init_pwf_adapter(pwf_z_coeff, pwf_p_coeff);
    pwf_z_coeff_next[0] = pwf_p_coeff_next[0] = 1.0;
    pwf_z_coeff_obsolete_p = 0;
    init_bsf_adapter(sf_coeff);
    sf_coeff_next[0] = 1.0;
    sf_coeff_obsolete_p = 0;
    init_gain_adapter(gp_coeff);
    gp_coeff_next[0] = 1.0;
    gp_coeff_obsolete_p = 0;
    init_input();
    vector_end=thequeue;
#ifdef __ADSP21000__
    mode2.b.cafrz = 0;
    interrupt(SIG_USR0, encode_vector);
#endif
}

void encode_vector(int ignore)
{
    int ix;	/* Computed Codebook Index */
    int vx;    	/* Index of Recently Read Vector  */
    int lgx;	/* Logarithmic Gain Index */

    static real QMEM *vector;	/* recently read vector in the queue */

    static real
	zero_response[IDIM],
	weighted_speech[IDIM],
	target[IDIM],
	normtarg[IDIM],
	cb_vec[IDIM],
	pn[IDIM];
    static real	gain =1.0, scale=1.0;
    
    vector = vector_end - IDIM;
    if (vector < thequeue)
	vector += QSIZE;
    vx = vector-thequeue;

    if (pwf_z_coeff_obsolete_p) {
	UPDATE(pwf_z_coeff); UPDATE(pwf_p_coeff);
    }
    				SPDEBUG(10,vector, IDIM);
    pwfilter2(vector, weighted_speech);
    				SPDEBUG(1,weighted_speech, IDIM);
    UPDATE(sf_coeff);
    zresp(zero_response);
    				SPDEBUG(2,zero_response, IDIM);
    sub_sig(weighted_speech, zero_response, target);
    				SPDEBUG(3,target, IDIM);
    scale = 1.0 / gain;
    sig_scale(scale, target, normtarg);
    				SPDEBUG(4,normtarg, IDIM);
    UPDATE(imp_resp);
    trev_conv(imp_resp, normtarg, pn);
    				SPDEBUG(6, pn, IDIM);
    UPDATE(shape_energy);
    ix = cb_index(pn);
    put_index(ix);
    cb_excitation(ix, cb_vec);
    				SPDEBUG(7, cb_vec, IDIM);
    sig_scale(gain, cb_vec, qspeech+vx);
    				SPDEBUG(8, qspeech+vx, IDIM);
    UPDATE(gp_coeff);
    lgx = vx/IDIM;
    gain = predict_gain(qspeech+vx, log_gains + lgx);
    				SPDEBUG(11, &gain, 1);
    				SPDEBUG(12, log_gains+lgx, 1);
    mem_update(qspeech+vx, synspeech+vx);
    				SPDEBUG(9, synspeech+vx, IDIM);
    dec_end = vx+IDIM;
    if (dec_end >= QSIZE)
	dec_end -= QSIZE;
    NEXT_FFASE;
}

void adapt_frame()
{
    static real
	input [NUPDATE*IDIM],
	synth [NUPDATE*IDIM],
	lg [NUPDATE];
    int gx;	/* Index for log_gains */
    
    FFASE(shifts[0])
    {
	CIRCOPY(synth,synspeech,dec_end,NUPDATE*IDIM,QSIZE);
	bsf_adapter (synth, sf_coeff_next);
	sf_coeff_obsolete_p = 1;
					SPDEBUG(15,sf_coeff, LPC+1);
    }

    FFASE(shifts[1])
    {
	gx = dec_end/IDIM;
	CIRCOPY(lg, log_gains, gx, NUPDATE, QSIZE/IDIM);
	gain_adapter(lg, gp_coeff_next);
	gp_coeff_obsolete_p = 1;
					SPDEBUG(13,gp_coeff, LPCLG+1);
    }

    FFASE(shifts[2])
    {
	CIRCOPY(input,thequeue,dec_end,NUPDATE*IDIM,QSIZE);
	pwf_adapter(input, pwf_z_coeff_next, pwf_p_coeff_next);
	pwf_z_coeff_obsolete_p = 1;
    }
	
    FFASE(shifts[3])
    {
	iresp_vcalc(sf_coeff_next, pwf_z_coeff_next, pwf_p_coeff_next, 
		    imp_resp_next);
	SPDEBUG(14,imp_resp, IDIM);
	shape_conv(imp_resp_next, shape_energy_next);
	shape_energy_obsolete_p = 1;
	imp_resp_obsolete_p = 1;
    }
}
