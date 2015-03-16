
static char const	ident[] = "$Header: /home/kbs/jutta/src/gsm/gsm-1.0/src/RCS/gsm_create.c,v 1.4 1996/07/02 09:59:05 jutta Exp $";

#include	"config.h"

#ifdef	HAS_STRING_H
#include	<string.h>
#else
#	include "proto.h"
	extern char	* memset P((char *, int, int));
#endif

#ifdef	HAS_STDLIB_H
#	include	<stdlib.h>
#else
#	ifdef	HAS_MALLOC_H
#		include 	<malloc.h>
#	else
		extern char * malloc();
#	endif
#endif

#include <stdio.h>

#include "gsm.h"
#include "private.h"
#include "proto.h"

gsm gsm_create P0()
{
	gsm  r;

	r = (gsm)malloc(sizeof(struct gsm_state));
	if (!r) return r;

	memset((char *)r, 0, sizeof(*r));
	r->nrp = 40;

	return r;
}
