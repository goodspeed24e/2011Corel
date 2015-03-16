#include <string.h>
#include	"config.h"


#ifdef	HAS_STDLIB_H
#include	<stdlib.h>
#else
#	include "proto.h"
	extern char	* memcpy P((char *, char *, int));
#endif

#include	"private.h"
#include	"gsm.h"
#include	"proto.h"

/* 
 *  4.2 FIXED POINT IMPLEMENTATION OF THE RPE-LTP CODER 
 */

void Gsm_Coder P8((S,s,LARc,Nc,bc,Mc,xmaxc,xMc),

	struct gsm_state	* S,
	word	* s,	/* [0..159] samples		  	IN	*/
	word	* LARc,	/* [0..7] LAR coefficients		OUT	*/
	word	* Nc,	/* [0..3] LTP lag			OUT 	*/
	word	* bc,	/* [0..3] coded LTP gain		OUT 	*/
	word	* Mc,	/* [0..3] RPE grid selection		OUT     */
	word	* xmaxc,/* [0..3] Coded maximum amplitude	OUT	*/
	word	* xMc	/* [13*4] normalized RPE samples	OUT	*/
)
{
	int	k;
	word	* dp  = S->dp0 + 120;	/* [ -120...-1 ] */
	word	* dpp = dp;		/* [ 0...39 ]	 */

	static word e[50];

	word	so[160];

	Gsm_Preprocess			(S, s, so);
	Gsm_LPC_Analysis		(S, so, LARc);
	Gsm_Short_Term_Analysis_Filter	(S, LARc, so);

	for (k = 0; k <= 3; k++, xMc += 13) {

		Gsm_Long_Term_Predictor	( S,
					 so+k*40, /* d      [0..39] IN	*/
					 dp,	  /* dp  [-120..-1] IN	*/
					e + 5,	  /* e      [0..39] OUT	*/
					dpp,	  /* dpp    [0..39] OUT */
					 Nc++,
					 bc++);

		Gsm_RPE_Encoding	( S,
					e + 5,	/* e	  ][0..39][ IN/OUT */
					  xmaxc++, Mc++, xMc );
		/*
		 * Gsm_Update_of_reconstructed_short_time_residual_signal
		 *			( dpp, e + 5, dp );
		 */

		{ register int i;
		  register longword ltmp;
		  for (i = 0; i <= 39; i++)
			dp[ i ] = GSM_ADD( e[5 + i], dpp[i] );
		}
		dp  += 40;
		dpp += 40;

	}
	(void)memcpy( (char *)S->dp0, (char *)(S->dp0 + 160),
		120 * sizeof(*S->dp0) );
}
