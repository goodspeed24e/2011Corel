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

#include <fcntl.h>
#include <stdio.h>
#include <multimedia/libaudio.h>
#include <multimedia/ulaw2linear.h>
#include "common.h"
#include "prototyp.h"

extern int fprintf();
extern void audio_read_filehdr();
extern int read(int, char*, int);
real rscale=0.1;  /* Scaling factor for input */


char * xfile_name;
#ifdef CODER

int oxfd = 0; /* output file (codebook indices) */
int ifd  = 1; /* input file */

char *ifile_name;

void
init_input()
{
    Audio_hdr hdr;
    int hdr_len = 100;
    
    if ((ifd=open(ifile_name, O_RDONLY)) < 0) {
	(void) fprintf(stderr, "Can't open \"%s\"\n", ifile_name);
	exit(1);
    }
    audio_read_filehdr(ifd, &hdr, 0, &hdr_len);

#ifdef TEST
    fprintf(stderr, "sample_rate\t= %d\n",	hdr.sample_rate);
    fprintf(stderr, "samples_per_unit\t= %d\n",	hdr.samples_per_unit);
    fprintf(stderr, "bytes_per_unit\t= %d\n",	hdr.bytes_per_unit);
    fprintf(stderr, "channels\t= %d\n",		hdr.channels);
    fprintf(stderr, "encoding\t= %d\n",		hdr.encoding);
    fprintf(stderr, "data_size\t= %d\n",	hdr.data_size);
#endif 
    
    if ((oxfd=open(xfile_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
	(void) fprintf(stderr, "Can't open \"%s\"\n", xfile_name);
    }
}

void
put_index(int x)
{
    short sx = x;
    write(oxfd, &sx, 2);
}

#endif

#ifdef DECODER
char * ofile_name;

static int ofd=1; /* Outpu file */
static ixfd = 0; /* Input file (codebook indices) */
int sound_overflow = 0;

void init_output()
{
    sound_overflow = 0;
    if ((ofd=open(ofile_name, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
	extern  int errno;
	extern char *sys_errlist[];
	int ee = errno;
	(void) fprintf(stderr, "Can't open \"%s\" for output\n", ofile_name);
	printf(sys_errlist[ee]);
	exit(1);
    }
    if ((ixfd = open(xfile_name, O_RDONLY)) < 0) {
	(void) fprintf(stderr, "Can't open \"%s\"\n", xfile_name);
	exit(3);
    }
}

int get_index()
{
    short sx;
    if (read(ixfd, (char*)&sx, sizeof(sx)) < sizeof(sx))
	return -1;
    return (int)sx;
}
#endif

/* Return Number of Samples Read */

#ifdef CODER

int read_sound_buffer(int n, real buf[])
{
    unsigned char ch;
    short s;
    int i, c=0;
    
    for (i=0; i<n && read(ifd, &ch, 1) > 0; i++) {
	s = audio_u2s(ch);
	buf[c++] =  rscale * (real) s;
    }
    return c;
}
#endif

#ifdef DECODER
int
write_sound_buffer(int n, real buf[])
{
    unsigned char ch;
    int i, c=0;
    
    for (i=0; i<n; i++) {
	float xx = buf[i]/rscale;
	short s;
	if (xx < - 0x7fff || xx > 0x7fff)
	    sound_overflow = 1;
	s = (short) xx;
	ch = audio_s2u(s);
	write(ofd, &ch, 1);
    }
    return c;
}
#endif
