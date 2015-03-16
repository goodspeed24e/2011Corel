#include	<stdio.h>
#include	<assert.h>

#include "private.h"

#include	"gsm.h"
#include 	"proto.h"


void Gsm_Preprocess P3((S, s, so),
	struct gsm_state * S,
	word		 * s,
	word 		 * so )		/* [0..159] 	IN/OUT	*/
{

	word       z1 = S->z1;
	longword L_z2 = S->L_z2;
	word 	   mp = S->mp;

	word 	   	s1;
	longword      L_s2;

	longword      L_temp;

	word		msp, lsp;
	word		SO;

	longword	ltmp;		/* for   ADD */
	ulongword	utmp;		/* for L_ADD */

	register int		k = 160;

	while (k--) {

		SO = SASR( *s, 3 ) << 2;
		s++;

		assert (SO >= -0x4000);	/* downscaled by     */
		assert (SO <=  0x3FFC);	/* previous routine. */


		s1 = SO - z1;			/* s1 = gsm_sub( *so, z1 ); */
		z1 = SO;

		assert(s1 != MIN_WORD);

		L_s2 = s1;
		L_s2 <<= 15;
		msp = SASR( L_z2, 15 );
		lsp = L_z2-((longword)msp<<15); /* gsm_L_sub(L_z2,(msp<<15)); */

		L_s2  += GSM_MULT_R( lsp, 32735 );
		L_temp = (longword)msp * 32735; /* GSM_L_MULT(msp,32735) >> 1;*/
		L_z2   = GSM_L_ADD( L_temp, L_s2 );

		L_temp = GSM_L_ADD( L_z2, 16384 );
		msp   = GSM_MULT_R( mp, -28180 );
		mp    = SASR( L_temp, 15 );
		*so++ = GSM_ADD( mp, msp );
	}

	S->z1   = z1;
	S->L_z2 = L_z2;
	S->mp   = mp;
}
