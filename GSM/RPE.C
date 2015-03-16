
#include <stdio.h>
#include <assert.h>

#include "private.h"

#include "gsm.h"
#include "proto.h"

static void Weighting_filter P2((e, x),
	register word	* e,		/* signal [-5..0.39.44]	IN  */
	word		* x		/* signal [0..39]	OUT */
)
{
	register longword	L_result;
	register int		k /* , i */ ;
	e -= 5;

	for (k = 0; k <= 39; k++) {

		L_result = 8192 >> 1;

#undef	STEP
#define	STEP( i, H )	(e[ k + i ] * (longword)H)


#ifdef	STUPID_COMPILER
		L_result += STEP(	0, 	-134 ) ;
		L_result += STEP(	1, 	-374 )  ;
		L_result += STEP(	3, 	2054 ) ;
		L_result += STEP(	4, 	5741 ) ;
		L_result += STEP(	5, 	8192 ) ;
		L_result += STEP(	6, 	5741 ) ;
		L_result += STEP(	7, 	2054 ) ;
	 	       /* + STEP(	8, 	0    )  */
		L_result += STEP(	9, 	-374 ) ;
		L_result += STEP(	10, 	-134 ) ;
#else
		L_result +=
		  STEP(	0, 	-134 ) 
		+ STEP(	1, 	-374 ) 
	     /* + STEP(	2, 	0    )  */
		+ STEP(	3, 	2054 ) 
		+ STEP(	4, 	5741 ) 
		+ STEP(	5, 	8192 ) 
		+ STEP(	6, 	5741 ) 
		+ STEP(	7, 	2054 ) 
	     /* + STEP(	8, 	0    )  */
		+ STEP(	9, 	-374 ) 
		+ STEP(10, 	-134 )
		;
#endif


		L_result = SASR( L_result, 13 );
		x[k] =  (  L_result < MIN_WORD ? MIN_WORD
			: (L_result > MAX_WORD ? MAX_WORD : L_result ));
	}
}

/* 4.2.14 */

static void RPE_grid_selection P3((x,xM,Mc_out),
	word		* x,		/* [0..39]		IN  */ 
	word		* xM,		/* [0..12]		OUT */
	word		* Mc_out	/*			OUT */
)
{
	/* register word	temp1;	*/
	register int		/* m, */  i;
	register longword	L_result, L_temp;
	longword		EM;	/* xxx should be L_EM? */
	word			Mc;

	longword		L_common_0_3;

	EM = 0;
	Mc = 0;

#undef	STEP
#define	STEP( m, i )		L_temp = SASR( x[m + 3 * i], 2 );	\
				L_result += L_temp * L_temp;


	L_result = 0;
	STEP( 0, 1 ); STEP( 0, 2 ); STEP( 0, 3 ); STEP( 0, 4 );
	STEP( 0, 5 ); STEP( 0, 6 ); STEP( 0, 7 ); STEP( 0, 8 );
	STEP( 0, 9 ); STEP( 0, 10); STEP( 0, 11); STEP( 0, 12);
	L_common_0_3 = L_result;


	STEP( 0, 0 );
	L_result <<= 1;	/* implicit in L_MULT */
	EM = L_result;


	L_result = 0;
	STEP( 1, 0 );
	STEP( 1, 1 ); STEP( 1, 2 ); STEP( 1, 3 ); STEP( 1, 4 );
	STEP( 1, 5 ); STEP( 1, 6 ); STEP( 1, 7 ); STEP( 1, 8 );
	STEP( 1, 9 ); STEP( 1, 10); STEP( 1, 11); STEP( 1, 12);
	L_result <<= 1;
	if (L_result > EM) {
		Mc = 1;
	 	EM = L_result;
	}


	L_result = 0;
	STEP( 2, 0 );
	STEP( 2, 1 ); STEP( 2, 2 ); STEP( 2, 3 ); STEP( 2, 4 );
	STEP( 2, 5 ); STEP( 2, 6 ); STEP( 2, 7 ); STEP( 2, 8 );
	STEP( 2, 9 ); STEP( 2, 10); STEP( 2, 11); STEP( 2, 12);
	L_result <<= 1;
	if (L_result > EM) {
		Mc = 2;
	 	EM = L_result;
	}


	L_result = L_common_0_3;
	STEP( 3, 12 );
	L_result <<= 1;
	if (L_result > EM) {
		Mc = 3;
	 	EM = L_result;
	}

	for (i = 0; i <= 12; i ++) xM[i] = x[Mc + 3*i];
	*Mc_out = Mc;
}


static void APCM_quantization_xmaxc_to_exp_mant P3((xmaxc,exp_out,mant_out),
	word		xmaxc,		/* IN 	*/
	word		* exp_out,	/* OUT	*/
	word		* mant_out )	/* OUT  */
{
	word	exp, mant;


	exp = 0;
	if (xmaxc > 15) exp = SASR(xmaxc, 3) - 1;
	mant = xmaxc - (exp << 3);

	if (mant == 0) {
		exp  = -4;
		mant = 7;
	}
	else {
		while (mant <= 7) {
			mant = mant << 1 | 1;
			exp--;
		}
		mant -= 8;
	}

	assert( exp  >= -4 && exp <= 6 );
	assert( mant >= 0 && mant <= 7 );

	*exp_out  = exp;
	*mant_out = mant;
}

static void APCM_quantization P5((xM,xMc,mant_out,exp_out,xmaxc_out),
	word		* xM,		/* [0..12]		IN	*/

	word		* xMc,		/* [0..12]		OUT	*/
	word		* mant_out,	/* 			OUT	*/
	word		* exp_out,	/*			OUT	*/
	word		* xmaxc_out	/*			OUT	*/
)
{
	int	i, itest;

	word	xmax, xmaxc, temp, temp1, temp2;
	word	exp, mant;


	xmax = 0;
	for (i = 0; i <= 12; i++) {
		temp = xM[i];
		temp = GSM_ABS(temp);
		if (temp > xmax) xmax = temp;
	}

	exp   = 0;
	temp  = SASR( xmax, 9 );
	itest = 0;

	for (i = 0; i <= 5; i++) {

		itest |= (temp <= 0);
		temp = SASR( temp, 1 );

		assert(exp <= 5);
		if (itest == 0) exp++;		/* exp = add (exp, 1) */
	}

	assert(exp <= 6 && exp >= 0);
	temp = exp + 5;

	assert(temp <= 11 && temp >= 0);
	xmaxc = gsm_add( SASR(xmax, temp), exp << 3 );
	APCM_quantization_xmaxc_to_exp_mant( xmaxc, &exp, &mant );

	assert( exp <= 4096 && exp >= -4096);
	assert( mant >= 0 && mant <= 7 ); 

	temp1 = 6 - exp;		/* normalization by the exponent */
	temp2 = gsm_NRFAC[ mant ];  	/* inverse mantissa 		 */

	for (i = 0; i <= 12; i++) {

		assert(temp1 >= 0 && temp1 < 16);

		temp = xM[i] << temp1;
		temp = GSM_MULT( temp, temp2 );
		temp = SASR(temp, 12);
		xMc[i] = temp + 4;		/* see note below */
	}
	*mant_out  = mant;
	*exp_out   = exp;
	*xmaxc_out = xmaxc;
}

static void APCM_inverse_quantization P4((xMc,mant,exp,xMp),
	register word	* xMc,	/* [0..12]			IN 	*/
	word		mant,
	word		exp,
	register word	* xMp)	/* [0..12]			OUT 	*/
{
	int	i;
	word	temp, temp1, temp2, temp3;
	longword	ltmp;

	assert( mant >= 0 && mant <= 7 ); 

	temp1 = gsm_FAC[ mant ];	/* see 4.2-15 for mant */
	temp2 = gsm_sub( 6, exp );	/* see 4.2-15 for exp  */
	temp3 = gsm_asl( 1, gsm_sub( temp2, 1 ));

	for (i = 13; i--;) {

		assert( *xMc <= 7 && *xMc >= 0 ); 	/* 3 bit unsigned */

		temp = (*xMc++ << 1) - 7;	        /* restore sign   */
		assert( temp <= 7 && temp >= -7 ); 	/* 4 bit signed   */

		temp <<= 12;				/* 16 bit signed  */
		temp = GSM_MULT_R( temp1, temp );
		temp = GSM_ADD( temp, temp3 );
		*xMp++ = gsm_asr( temp, temp2 );
	}
}

/* 4.2.17 */

static void RPE_grid_positioning P3((Mc,xMp,ep),
	word		Mc,		/* grid position	IN	*/
	register word	* xMp,		/* [0..12]		IN	*/
	register word	* ep		/* [0..39]		OUT	*/
)
{
	int	i = 13;

	assert(0 <= Mc && Mc <= 3);

        switch (Mc) {
                case 3: *ep++ = 0;
                case 2:  do {
                                *ep++ = 0;
                case 1:         *ep++ = 0;
                case 0:         *ep++ = *xMp++;
                         } while (--i);
        }
        while (++Mc < 4) *ep++ = 0;
}

#if 0	/* Has been inlined in code.c */
void Gsm_Update_of_reconstructed_short_time_residual_signal P3((dpp, ep, dp),
	word	* dpp,		/* [0...39]	IN	*/
	word	* ep,		/* [0...39]	IN	*/
	word	* dp)		/* [-120...-1]  IN/OUT 	*/
{
	int 		k;

	for (k = 0; k <= 79; k++) 
		dp[ -120 + k ] = dp[ -80 + k ];

	for (k = 0; k <= 39; k++)
		dp[ -40 + k ] = gsm_add( ep[k], dpp[k] );
}
#endif	/* Has been inlined in code.c */

void Gsm_RPE_Encoding P5((S,e,xmaxc,Mc,xMc),

	struct gsm_state * S,

	word	* e,		/* -5..-1][0..39][40..44	IN/OUT  */
	word	* xmaxc,	/* 				OUT */
	word	* Mc,		/* 			  	OUT */
	word	* xMc)		/* [0..12]			OUT */
{
	word	x[40];
	word	xM[13], xMp[13];
	word	mant, exp;

	Weighting_filter(e, x);
	RPE_grid_selection(x, xM, Mc);

	APCM_quantization(	xM, xMc, &mant, &exp, xmaxc);
	APCM_inverse_quantization(  xMc,  mant,  exp, xMp);

	RPE_grid_positioning( *Mc, xMp, e );

}

void Gsm_RPE_Decoding P5((S, xmaxcr, Mcr, xMcr, erp),
	struct gsm_state	* S,

	word 		xmaxcr,
	word		Mcr,
	word		* xMcr,  /* [0..12], 3 bits 		IN	*/
	word		* erp	 /* [0..39]			OUT 	*/
)
{
	word	exp, mant;
	word	xMp[ 13 ];

	APCM_quantization_xmaxc_to_exp_mant( xmaxcr, &exp, &mant );
	APCM_inverse_quantization( xMcr, mant, exp, xMp );
	RPE_grid_positioning( Mcr, xMp, erp );

}
