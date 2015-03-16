
#include	"toast.h"
extern FILE	*in, *out;

int linear_input (buf) gsm_signal * buf;
{
	return fread( (char *)buf, sizeof(*buf), 160, in );
}

int linear_output P1((buf), gsm_signal * buf) 
{
	return -( fwrite( (char *)buf, sizeof(*buf), 160, out ) != 160 );
}
