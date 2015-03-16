/*--------------------------------------------------------------------------------*
 *                                                                                *
 * This material is trade secret owned by imtelephone.com                         *
 * and is strictly confidential and shall remain as such.                         *
 *                                                                                *
 * Copyright ?2003-2004 imtelephone.com. All Rights Reserved. No part of         *
 * this material may be reproduced, stored in a retrieval system, or transmitted, *
 * in any form or by any means, including, but not limited to, photocopying,      *
 *  electronic, mechanical, recording, or otherwise, without the prior written    *
 * permission of imtelephone.com.                                                 *
 *                                                                                *
 * This material is subject to continuous developments and improvements. All      *
 * warranties implied or expressed, including but not limited to implied          *
 * warranties of merchantability, or fitness for purpose, are excluded.           *
 *                                                                                *
 *--------------------------------------------------------------------------------*
 *                                                                                *
 * support@imtelephone.com                                                        *
 *                                                                                *
 *--------------------------------------------------------------------------------*
 *
 *--------------------------------------------------------------------------------*
 *                         EasyG729A_decoder.c                                    *
 *                         ~~~~~~~~~~~~~~~~~~                                     *
 * Example of the ITU G.729A 8.0kbps speech decoder                              *
 *                                                                                *
 *--------------------------------------------------------------------------------*/
 

#include "stdio.h"
#include <windows.h>
#include "EasyG729A.h"
#include <time.h>

void main(int argc, char *argv[])
{

	int nb_frame;

	clock_t start, finish;
	double duration;

	FILE* fp_in;
	FILE* fp_out;

	unsigned char	serial[L_G729A_FRAME_COMPRESSED];
	short			synth[L_G729A_FRAME];

	CODER_HANDLE hDecoder;

	printf("\n**************         Imtelephone.com        **************");
	printf("\n");
	printf("\n-------------      G729A Decoder      ------------");
	printf("\n");

	/*-----------------------------------------------------------------------*
	 * Open all files.                                                       *
	 *-----------------------------------------------------------------------*/

	if (argc != 3)
	{
		printf("Usage: %s infile outfile\n", argv[0]);
		return;
	}

	if ( (fp_in = fopen(argv[1], "rb")) == NULL)
	{
		printf("\nError opening input file %s!", argv[1]);
		return;
	} 

	if ( (fp_out = fopen(argv[2], "wb")) == NULL)
	{
		printf("\nError opening output file %s!", argv[2]);
		return;
	}

	/*-----------------------------------------------------------------------*
	 * Decode                                                                *
	 *-----------------------------------------------------------------------*/

	hDecoder = EasyG729A_init_decoder(  );

	nb_frame = 0;
  	start = clock();

	while (fread(serial, sizeof(char), L_G729A_FRAME_COMPRESSED, fp_in) == L_G729A_FRAME_COMPRESSED) 
	{

		printf("Decode frame %d\r", ++nb_frame);

		/*--------------------------------------------------------------*
		 * Call the decoder.                                            *
		 *--------------------------------------------------------------*/

		EasyG729A_decoder(hDecoder, serial, synth );

		/*--------------------------------------------------------------*
		 * Output synthesis to disk                                     *
		 *--------------------------------------------------------------*/

		fwrite(synth, sizeof(short), L_G729A_FRAME, fp_out);
	}

   EasyG729A_release_decoder( hDecoder );
 
   finish = clock();
   
   duration = (double)(finish - start) / CLOCKS_PER_SEC;
   printf( "\n%2.1f seconds\n", duration );

	fclose(fp_out);
	fclose(fp_in);

} /* end of main() */
