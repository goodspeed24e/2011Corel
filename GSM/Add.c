
#include	<stdio.h>
#include	<assert.h>

#include	"private.h"
#include	"gsm.h"
#include	"proto.h"

#define	saturate(x) 	\
	((x) < MIN_WORD ? MIN_WORD : (x) > MAX_WORD ? MAX_WORD: (x))

word gsm_add P2((a,b), word a, word b)
{
	longword sum = (longword)a + (longword)b;
	return saturate(sum);
}

word gsm_sub P2((a,b), word a, word b)
{
	longword diff = (longword)a - (longword)b;
	return saturate(diff);
}

word gsm_mult P2((a,b), word a, word b)
{
	if (a == MIN_WORD && b == MIN_WORD) return MAX_WORD;
	else return SASR( (longword)a * (longword)b, 15 );
}

word gsm_mult_r P2((a,b), word a, word b)
{
	if (b == MIN_WORD && a == MIN_WORD) return MAX_WORD;
	else {
		longword prod = (longword)a * (longword)b + 16384;
		prod >>= 15;
		return prod & 0xFFFF;
	}
}

word gsm_abs P1((a), word a)
{
	return a < 0 ? (a == MIN_WORD ? MAX_WORD : -a) : a;
}

longword gsm_L_mult P2((a,b),word a, word b)
{
	assert( a != MIN_WORD || b != MIN_WORD );
	return ((longword)a * (longword)b) << 1;
}

longword gsm_L_add P2((a,b), longword a, longword b)
{
	if (a < 0) {
		if (b >= 0) return a + b;
		else {
			ulongword A = (ulongword)-(a + 1) + (ulongword)-(b + 1);
			return A >= MAX_LONGWORD ? MIN_LONGWORD :-(longword)A-2;
		}
	}
	else if (b <= 0) return a + b;
	else {
		ulongword A = (ulongword)a + (ulongword)b;
		return A > MAX_LONGWORD ? MAX_LONGWORD : A;
	}
}

longword gsm_L_sub P2((a,b), longword a, longword b)
{
	if (a >= 0) {
		if (b >= 0) return a - b;
		else {
			/* a>=0, b<0 */

			ulongword A = (ulongword)a + -(b + 1);
			return A >= MAX_LONGWORD ? MAX_LONGWORD : (A + 1);
		}
	}
	else if (b <= 0) return a - b;
	else {
		/* a<0, b>0 */  

		ulongword A = (ulongword)-(a + 1) + b;
		return A >= MAX_LONGWORD ? MIN_LONGWORD : -(longword)A - 1;
	}
}

static unsigned char const bitoff[ 256 ] = {
	 8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
	 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

word gsm_norm P1((a), longword a )
{
	assert(a != 0);

	if (a < 0) {
		if (a <= -1073741824) return 0;
		a = ~a;
	}

	return    a & 0xffff0000 
		? ( a & 0xff000000
		  ?  -1 + bitoff[ 0xFF & (a >> 24) ]
		  :   7 + bitoff[ 0xFF & (a >> 16) ] )
		: ( a & 0xff00
		  ?  15 + bitoff[ 0xFF & (a >> 8) ]
		  :  23 + bitoff[ 0xFF & a ] );
}

longword gsm_L_asl P2((a,n), longword a, int n)
{
	if (n >= 32) return 0;
	if (n <= -32) return -(a < 0);
	if (n < 0) return gsm_L_asr(a, -n);
	return a << n;
}

word gsm_asl P2((a,n), word a, int n)
{
	if (n >= 16) return 0;
	if (n <= -16) return -(a < 0);
	if (n < 0) return gsm_asr(a, -n);
	return a << n;
}

longword gsm_L_asr P2((a,n), longword a, int n)
{
	if (n >= 32) return -(a < 0);
	if (n <= -32) return 0;
	if (n < 0) return a << -n;

#	ifdef	SASR
		return a >> n;
#	else
		if (a >= 0) return a >> n;
		else return -(longword)( -(ulongword)a >> n );
#	endif
}

word gsm_asr P2((a,n), word a, int n)
{
	if (n >= 16) return -(a < 0);
	if (n <= -16) return 0;
	if (n < 0) return a << -n;

#	ifdef	SASR
		return a >> n;
#	else
		if (a >= 0) return a >> n;
		else return -(word)( -(uword)a >> n );
#	endif
}

word gsm_div P2((num,denum), word num, word denum)
{
	longword	L_num   = num;
	longword	L_denum = denum;
	word		div 	= 0;
	int		k 	= 15;

	/* The parameter num sometimes becomes zero.
	 * Although this is explicitly guarded against in 4.2.5,
	 * we assume that the result should then be zero as well.
	 */

	/* assert(num != 0); */

	assert(num >= 0 && denum >= num);
	if (num == 0)
	    return 0;

	while (k--) {
		div   <<= 1;
		L_num <<= 1;

		if (L_num >= L_denum) {
			L_num -= L_denum;
			div++;
		}
	}

	return div;
}
