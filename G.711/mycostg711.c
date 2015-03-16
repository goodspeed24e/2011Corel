
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "g711.c"

/* 16 bit swapping */
short swap_linear (short pcm_val)
{ struct lohibyte { unsigned char lb, hb;}; 
  union { struct lohibyte b;
          short i;
        } exchange;
  unsigned char c;
  
  exchange.i      = pcm_val;
  c               = exchange.b.hb;
  exchange.b.hb   = exchange.b.lb; 
  exchange.b.lb   = c;
  return (exchange.i);
}
/*
 **************************************************************************
 **************************************************************************
*/
void PrintUsage (FILE *ef)
{ fprintf(ef, "CCITT G.711 u-law, A-law and linear PCM conversions.\n");
  fprintf(ef, "          Version 94/12/30 - COST 232\n");
  fprintf(ef, "\nUsage:\n\n");
  fprintf(ef, "\tcostg711 -au|ua|la|al|lu|ul|ll infile outfile\n\n");
  fprintf(ef, "\t\t-au\tA-law to u-law conversion\n");
  fprintf(ef, "\t\t-ua\tu-law to A-law conversion\n");
  fprintf(ef, "\t\t-la\t16-bit linear PCM to A-law\n");
  fprintf(ef, "\t\t-al\tA-law to 16-bit linear PCM\n"); 
  fprintf(ef, "\t\t-lu\t16-bit linear PCM to u-law\n");
  fprintf(ef, "\t\t-ul\tu-law to 16-bit linear PCM\n"); 
  fprintf(ef, "\t\t-ll\t16-bit linear low/high byte swapping\n"); 
}

/*
 **************************************************************************
 **************************************************************************
*/
#define BLOCK_SIZE 1024
union
{ unsigned char in_samples_char[BLOCK_SIZE];
  short         in_samples_short[BLOCK_SIZE];
} inbuf;

union
{ unsigned char out_samples_char[BLOCK_SIZE];
  short         out_samples_short[BLOCK_SIZE];
} outbuf;

int main()
{
        short                   in_size, out_size, count;
        short                   (*char_short_routine)(unsigned char uval) = NULL;
        short                   (*short_short_routine)(short pcm_val) = NULL;
        unsigned char           (*char_char_routine)(unsigned char uval) = NULL;
        unsigned char           (*short_char_routine)(short     pcm_val) = NULL;
        FILE                    *infile, *outfile;
		char  ch_1,ch_2,name1[30],name2[30];
		printf(" input first&second char\n");
        scanf("%c%c",&ch_1,&ch_2);
		printf(" input source name\n");
        scanf("%s",name1);
		printf(" input result name\n");
        scanf("%s",name2);


        out_size = sizeof (char);
        in_size =  sizeof (char);
        switch (ch_1) {
        case 'u':
                switch (ch_2) {
                case 'a':
                        char_char_routine = ulaw2alaw;
                        break;
                case 'l':
                        out_size = sizeof (short);
                        char_short_routine = ulaw2linear;
                        break;
                }
                break;
        case 'a':
                switch (ch_2) {
                case 'u':
                        char_char_routine = alaw2ulaw;
                        break;
                case 'l':
                        out_size = sizeof (short);
                        char_short_routine = alaw2linear;
                        break;
                }
                break;
        case 'l':
                in_size = sizeof (short);
                switch (ch_2) {
                case 'u':
                        short_char_routine = linear2ulaw;
                        break;
                case 'a':
                        short_char_routine = linear2alaw;
                        break;
                case 'l':
                        out_size = sizeof (short);
                        short_short_routine = swap_linear;
                        break;
                }
                break;
        default:
                PrintUsage(stderr); return 1; /* Exit, error code 1 */
        }

        if ((infile = fopen(name1,"rb")) == NULL)
        { fprintf(stderr,"Cannot open input file %s\n",name1);
          return 1; /* Exit, error code 1 */
        }
        
        if ((outfile = fopen(name2,"wb")) == NULL)
        { fprintf(stderr,"Cannot open output file %s\n",name2);
          return 1; /* Exit, error code 1 */
        }
        
        /* Read input file and process */
        do
        {  short i;

           count = fread(&inbuf,in_size,BLOCK_SIZE,infile);
           switch (out_size) {
                
                case 1: switch(in_size) {
                        case 1: for (i=0; i < count; i++)
                                  outbuf.out_samples_char[i] = (*char_char_routine) 
                                    (inbuf.in_samples_char[i]);
                                break;
                        case 2: for (i=0; i < count; i++)
                                  outbuf.out_samples_char[i] = (*short_char_routine) 
                                    (inbuf.in_samples_short[i]);
                                break;
                        }
                        break;
                        
                case 2: switch(in_size) {
                        case 1: for (i=0; i < count; i++)
                                  outbuf.out_samples_short[i] = (*char_short_routine) 
                                    (inbuf.in_samples_char[i]);
                                break;
                        case 2: for (i=0; i < count; i++)
                                  outbuf.out_samples_short[i] = (*short_short_routine) 
                                    (inbuf.in_samples_short[i]);
                                break;
                        }
                        break;
                        
           } /* end switch */
           if (fwrite(&outbuf, out_size, count, outfile) != count)
             { fprintf(stderr,"Write Access Error, File=%s",name2); return 1;}
        } while (count == BLOCK_SIZE);

        fclose(infile); 
        fclose(outfile);

        return 0; /* Exit, no errors */
}
