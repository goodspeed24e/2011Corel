
#include	"toast.h"

char   * progname;

int	f_decode   = 1;		/* decode rather than encode	 (-d) */
int 	f_cat	   = 0;		/* write to stdout; implies -p   (-c) */
int	f_force	   = 0;		/* don't ask about replacements  (-f) */
int	f_precious = 0;		/* avoid deletion of original	 (-p) */
int	f_fast	   = 0;		/* use faster fpt algorithm	 (-F) */
int	f_verbose  = 0;		/* debugging			 (-V) */
int	f_ltp_cut  = 0;		/* LTP cut-off margin	      	 (-C) */



FILE	*in, 	 *out;
char	inname[30], outname[30];

/*
 *  The function (*output)() writes a frame of 160 samples given as
 *  160 signed 16 bit values (gsm_signals) to <out>.
 *  The function (*input)() reads one such frame from <in>.
 *  The function (*init_output)() begins output (e.g. writes a header).,
 *  The function (*init_input)() begins input (e.g. skips a header).
 *
 *  There are different versions of input, output, init_input and init_output
 *  for different formats understood by toast; which ones are used 
 *  depends on the command line arguments and, in their absence, the
 *  filename; the fallback is #defined in toast.h
 *
 *  The specific implementations of input, output, init_input and init_output
 *  for a format `foo' live in toast_foo.c.
*/

int	(*output   ) P((gsm_signal *)),
	(*input    ) P((gsm_signal *));
int	(*init_input)  P((void)),
	(*init_output) P((void));

static int	generic_init P0() { return 0; }	/* NOP */

struct fmtdesc {

	char * name, * longname, * suffix;

	int  (* init_input )  P((void)),
	     (* init_output)  P((void));

	int  (* input ) P((gsm_signal * )),
	     (* output) P((gsm_signal * ));

}  f_linear = {
		"linear",
		"16 bit (13 significant) signed 8 kHz signal", ".l",
		generic_init,
		generic_init,
		linear_input,
		linear_output
};

struct fmtdesc * alldescs[] = {
	&f_linear,
	(struct fmtdesc *)NULL
};

#define	DEFAULT_FORMAT	f_linear	/* default audio format, others	*/
					/* are: f_alaw,f_audio,f_linear */
struct fmtdesc * f_format  = 0;




static void prepare_io P1(( desc), struct fmtdesc * desc)
{
	output      = desc->output;
	input       = desc->input;

	init_input  = desc->init_input;
	init_output = desc->init_output;
}





static int process_encode P0()
{
	gsm      	r;
	gsm_signal    	s[ 160 ];
	gsm_frame	d;
 
	int		cc;

	if (!(r = gsm_create())) {
		perror(progname);
		return -1;
	}
	(void)gsm_option(r, GSM_OPT_FAST,       &f_fast);
	(void)gsm_option(r, GSM_OPT_VERBOSE,    &f_verbose);
	(void)gsm_option(r, GSM_OPT_LTP_CUT,	&f_ltp_cut);		
	//while ((cc = (*input)(s)) > 0) {
	while ((cc = fread(s,2,160,in)) > 0) {

		if (cc < sizeof(s) / sizeof(*s))
			memset((char *)(s+cc), 0, sizeof(s)-(cc * sizeof(*s)));

		gsm_encode(r, s, d);
		if (fwrite((char *)d, sizeof(d), 1, out) != 1) {
			perror(outname ? outname : "stdout");
			fprintf(stderr, "%s: error writing to %s\n",
				progname, outname ? outname : "stdout");
			gsm_destroy(r);
			return -1;
		}
	}
	if (cc < 0) {
		perror(inname ? inname : "stdin");
		fprintf(stderr, "%s: error reading from %s\n",
			progname, inname ? inname : "stdin");
		gsm_destroy(r);
		return -1;
	}
	gsm_destroy(r);

	return 0;
}

static int process_decode P0()
{
	gsm      	r;
	gsm_frame	s;
	gsm_signal	d[ 160 ];
 
	int		cc;

	if (!(r = gsm_create())) {	/* malloc failed */
		perror(progname);
		return -1;
	}
	(void)gsm_option(r, GSM_OPT_FAST,    &f_fast);
	(void)gsm_option(r, GSM_OPT_VERBOSE, &f_verbose);

		while ((cc = fread(s, 1, sizeof(s), in)) > 0) {
			//erroreffect(s);

		if (cc != sizeof(s)) {
			if (cc >= 0) fprintf(stderr,
			"%s: incomplete frame (%d byte%s missing) from %s\n",
					progname, sizeof(s) - cc,
					"s" + (sizeof(s) - cc == 1),
					inname ? inname : "stdin" );
			gsm_destroy(r);
			errno = 0;
			return -1;
		}
		if (gsm_decode(r, s, d)) {
			fprintf(stderr, "%s: bad frame in %s\n", 
				progname, inname ? inname : "stdin");
			gsm_destroy(r);
			errno = 0;
			return -1;
		}

		//if ((*output)(d) < 0) {
		if (fwrite(d,2,160,out) < 0) {
			perror(outname);
			fprintf(stderr, "%s: error writing to %s\n",
					progname, outname);
			gsm_destroy(r);
			return -1;
		}
	}

	if (cc < 0) {
		perror(inname ? inname : "stdin" );
		fprintf(stderr, "%s: error reading from %s\n", progname,
			inname ? inname : "stdin");
		gsm_destroy(r);
		return -1;
	}

	gsm_destroy(r);
	return 0;
}


int main ()
{

	int flag=0;
	int control_flag;
	printf("\n input control flag=<int put=0 encode&decode outsame?>\n<int put=1 encode outsame?>\n<int put=2 decode outsame?>\n");
	scanf("%d",&control_flag);  printf("control_flag=%d\n",control_flag);
if(control_flag!=2)
{	
	printf("\n input insame?\n");
	scanf("%s",inname);
	printf("\n input outsame?\n");
	scanf("%s",outname);

	in = fopen (inname , "rb" );
	out = fopen ( outname, "wb" );
	prepare_io ( f_linear );	
	process_encode ();
	fclose ( in );
	fclose ( out );
}
if(control_flag!=1)
{	
	printf("\n input insame?\n");
	scanf("%s",inname);
	printf("\n input outsame?\n");
	scanf("%s",outname);

	in = fopen (inname, "rb" );  //码流文件264反复
	out = fopen ( outname, "wb" );
	prepare_io ( f_linear );	
	process_decode ();
	fclose ( in );
	fclose ( out );

}
}
