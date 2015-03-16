
/*!
*************************************************************************************
* \file biaridecod.c
*
* \brief
*    binary arithmetic decoder routines
* \date
*    21. Oct 2000
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Detlev Marpe                    <marpe@hhi.de>
*    - Gabi Blaettermann               <blaetter@hhi.de>
*************************************************************************************
*/

#include <stdlib.h>
#include "global.h"
#include "biaridecod.h"

// Disable "no EMMS" warning
#pragma warning ( disable : 4799 )

#ifdef _GLOBAL_IMG_
#define DPP img.g_dep.
#else
#define DPP img->g_dep.
#endif

#define _COMBINED_STATE_BIT_

#define Dep_offset 0

#define Dcodestrm		DPP Dcodestrm
#define Dbits_to_go		DPP	Dbits_to_go
#define Dbuffer			DPP Dbuffer
#define Drange			DPP Drange
#define Ddelta			DPP Ddelta

#define B_BITS    10  // Number of bits to represent the whole coding interval
#define HALF      (1 << (B_BITS-1))
#define QUARTER   (1 << (B_BITS-2))

void arideco_start_decoding PARGS0()
{
	unsigned int *codestrm = (unsigned int *) Dcodestrm;
#ifdef _BIG_ENDIAN_
	Dbuffer    = *(int *) &Dcodestrm[0];
#else
	//Dbuffer    = Dcodestrm[0]<<24 | Dcodestrm[1]<<16 | Dcodestrm[2]<<8 | Dcodestrm[3];
	Dbuffer      = _bswap(*codestrm);
#endif
	Dcodestrm += 4;
	Dbits_to_go = 32-(B_BITS-1);
	Drange     = HALF-2;
	Ddelta     = (Dbuffer>>Dbits_to_go) - Drange;
	Dbuffer  <<= (B_BITS-1);
}

DecodingEnvironment *arideco_get_dep PARGS0()
{
#ifdef _GLOBAL_IMG_
	return &(img.g_dep);
#else
	return &(img->g_dep);
#endif
}

void biari_init_context PARGS2(BiContextTypePtr ctx, const int *ini)
{
	int pstate;

	pstate = (ini[0]*(IMGPAR qp-((int) IMGPAR qp>>31)&IMGPAR qp) >> 4) + ini[1];
#ifdef _ARM_FIX_
	*ctx = __fast_iclip(1,126,pstate);
#else
	ctx->state = __fast_iclip(1,126,pstate);
#endif
}


// state = (MSB | state) (0..127)
// First 4 entries are rLPS (depends on 2 MSBs of range)
// Second 2 entries are next state (depends on internal decision)
// Last 2 entries are return bit (depends on internal decision)
extern "C" const unsigned char __declspec(align(32)) total_table[128*8] GCC_ALIGN(32) =
{
	2,   2,   2,   2,   0,   0,   0,   1,
	6,   7,   8,   9,   1,  25,   0,   1,
	6,   7,   9,  10,   1,  25,   0,   1,
	6,   8,   9,  11,   2,  26,   0,   1,
	7,   8,  10,  11,   3,  26,   0,   1,
	7,   9,  10,  12,   4,  26,   0,   1,
	7,   9,  11,  12,   5,  27,   0,   1,
	8,   9,  11,  13,   6,  27,   0,   1,
	8,  10,  12,  14,   7,  27,   0,   1,
	9,  11,  12,  14,   8,  28,   0,   1,
	9,  11,  13,  15,   9,  28,   0,   1,
	10,  12,  14,  16,  10,  28,   0,   1,
	10,  12,  15,  17,  11,  29,   0,   1,
	11,  13,  15,  18,  12,  29,   0,   1,
	11,  14,  16,  19,  13,  30,   0,   1,
	12,  14,  17,  20,  14,  30,   0,   1,
	12,  15,  18,  21,  15,  30,   0,   1,
	13,  16,  19,  22,  16,  31,   0,   1,
	14,  17,  20,  23,  17,  31,   0,   1,
	14,  18,  21,  24,  18,  32,   0,   1,
	15,  19,  22,  25,  19,  33,   0,   1,
	16,  20,  23,  27,  20,  33,   0,   1,
	17,  21,  25,  28,  21,  33,   0,   1,
	18,  22,  26,  30,  22,  34,   0,   1,
	19,  23,  27,  31,  23,  34,   0,   1,
	20,  24,  29,  33,  24,  35,   0,   1,
	21,  26,  30,  35,  25,  36,   0,   1,
	22,  27,  32,  37,  26,  36,   0,   1,
	23,  28,  33,  39,  27,  37,   0,   1,
	24,  30,  35,  41,  28,  37,   0,   1,
	26,  31,  37,  43,  29,  38,   0,   1,
	27,  33,  39,  45,  30,  39,   0,   1,
	29,  35,  41,  48,  31,  39,   0,   1,
	30,  37,  43,  50,  32,  40,   0,   1,
	32,  39,  46,  53,  33,  41,   0,   1,
	33,  41,  48,  56,  34,  41,   0,   1,
	35,  43,  51,  59,  35,  42,   0,   1,
	37,  45,  54,  62,  36,  42,   0,   1,
	39,  48,  56,  65,  37,  44,   0,   1,
	41,  50,  59,  69,  38,  44,   0,   1,
	43,  53,  63,  72,  39,  45,   0,   1,
	46,  56,  66,  76,  40,  45,   0,   1,
	48,  59,  69,  80,  41,  47,   0,   1,
	51,  62,  73,  85,  42,  47,   0,   1,
	53,  65,  77,  89,  43,  48,   0,   1,
	56,  69,  81,  94,  44,  48,   0,   1,
	59,  72,  86,  99,  45,  50,   0,   1,
	62,  76,  90, 104,  46,  50,   0,   1,
	66,  80,  95, 110,  47,  51,   0,   1,
	69,  85, 100, 116,  48,  52,   0,   1,
	73,  89, 105, 122,  49,  52,   0,   1,
	77,  94, 111, 128,  50,  54,   0,   1,
	81,  99, 117, 135,  51,  54,   0,   1,
	85, 104, 123, 142,  52,  55,   0,   1,
	90, 110, 130, 150,  53,  56,   0,   1,
	95, 116, 137, 158,  54,  57,   0,   1,
	100, 122, 144, 166,  55,  58,   0,   1,
	105, 128, 152, 175,  56,  59,   0,   1,
	111, 135, 160, 185,  57,  59,   0,   1,
	116, 142, 169, 195,  58,  61,   0,   1,
	123, 150, 178, 205,  59,  61,   0,   1,
	128, 158, 187, 216,  60,  62,   0,   1,
	128, 167, 197, 227,  61,  63,   0,   1,
	128, 176, 208, 240,  62,  64,   0,   1,
	128, 176, 208, 240,  65,  63,   1,   0,
	128, 167, 197, 227,  66,  64,   1,   0,
	128, 158, 187, 216,  67,  65,   1,   0,
	123, 150, 178, 205,  68,  66,   1,   0,
	116, 142, 169, 195,  69,  66,   1,   0,
	111, 135, 160, 185,  70,  68,   1,   0,
	105, 128, 152, 175,  71,  68,   1,   0,
	100, 122, 144, 166,  72,  69,   1,   0,
	95, 116, 137, 158,  73,  70,   1,   0,
	90, 110, 130, 150,  74,  71,   1,   0,
	85, 104, 123, 142,  75,  72,   1,   0,
	81,  99, 117, 135,  76,  73,   1,   0,
	77,  94, 111, 128,  77,  73,   1,   0,
	73,  89, 105, 122,  78,  75,   1,   0,
	69,  85, 100, 116,  79,  75,   1,   0,
	66,  80,  95, 110,  80,  76,   1,   0,
	62,  76,  90, 104,  81,  77,   1,   0,
	59,  72,  86,  99,  82,  77,   1,   0,
	56,  69,  81,  94,  83,  79,   1,   0,
	53,  65,  77,  89,  84,  79,   1,   0,
	51,  62,  73,  85,  85,  80,   1,   0,
	48,  59,  69,  80,  86,  80,   1,   0,
	46,  56,  66,  76,  87,  82,   1,   0,
	43,  53,  63,  72,  88,  82,   1,   0,
	41,  50,  59,  69,  89,  83,   1,   0,
	39,  48,  56,  65,  90,  83,   1,   0,
	37,  45,  54,  62,  91,  85,   1,   0,
	35,  43,  51,  59,  92,  85,   1,   0,
	33,  41,  48,  56,  93,  86,   1,   0,
	32,  39,  46,  53,  94,  86,   1,   0,
	30,  37,  43,  50,  95,  87,   1,   0,
	29,  35,  41,  48,  96,  88,   1,   0,
	27,  33,  39,  45,  97,  88,   1,   0,
	26,  31,  37,  43,  98,  89,   1,   0,
	24,  30,  35,  41,  99,  90,   1,   0,
	23,  28,  33,  39, 100,  90,   1,   0,
	22,  27,  32,  37, 101,  91,   1,   0,
	21,  26,  30,  35, 102,  91,   1,   0,
	20,  24,  29,  33, 103,  92,   1,   0,
	19,  23,  27,  31, 104,  93,   1,   0,
	18,  22,  26,  30, 105,  93,   1,   0,
	17,  21,  25,  28, 106,  94,   1,   0,
	16,  20,  23,  27, 107,  94,   1,   0,
	15,  19,  22,  25, 108,  94,   1,   0,
	14,  18,  21,  24, 109,  95,   1,   0,
	14,  17,  20,  23, 110,  96,   1,   0,
	13,  16,  19,  22, 111,  96,   1,   0,
	12,  15,  18,  21, 112,  97,   1,   0,
	12,  14,  17,  20, 113,  97,   1,   0,
	11,  14,  16,  19, 114,  97,   1,   0,
	11,  13,  15,  18, 115,  98,   1,   0,
	10,  12,  15,  17, 116,  98,   1,   0,
	10,  12,  14,  16, 117,  99,   1,   0,
	9,  11,  13,  15, 118,  99,   1,   0,
	9,  11,  12,  14, 119,  99,   1,   0,
	8,  10,  12,  14, 120, 100,   1,   0,
	8,   9,  11,  13, 121, 100,   1,   0,
	7,   9,  11,  12, 122, 100,   1,   0,
	7,   9,  10,  12, 123, 101,   1,   0,
	7,   8,  10,  11, 124, 101,   1,   0,
	6,   8,   9,  11, 125, 101,   1,   0,
	6,   7,   9,  10, 126, 102,   1,   0,
	6,   7,   8,   9, 126, 102,   1,   0,
	2,   2,   2,   2, 127, 127,   1,   0,
};


/* Range table for  LPS */ 
/*
extern "C" const unsigned char __declspec(align(32)) rLPS_table_4x64[256] GCC_ALIGN(32) =
{
128,128,128,123,116,111,105,100,
95, 90, 85, 81, 77, 73, 69, 66,
62, 59, 56, 53, 51, 48, 46, 43,
41, 39, 37, 35, 33, 32, 30, 29,
27, 26, 24, 23, 22, 21, 20, 19,
18, 17, 16, 15, 14, 14, 13, 12,
12, 11, 11, 10, 10,  9,  9,  8,
8,  7,  7,  7,  6,  6,  6,  2,
176,167,158,150,142,135,128,122,
116,110,104, 99, 94, 89, 85, 80,
76, 72, 69, 65, 62, 59, 56, 53,
50, 48, 45, 43, 41, 39, 37, 35,
33, 31, 30, 28, 27, 26, 24, 23,
22, 21, 20, 19, 18, 17, 16, 15,
14, 14, 13, 12, 12, 11, 11, 10,
9,  9,  9,  8,  8,  7,  7,  2,
208,197,187,178,169,160,152,144,
137,130,123,117,111,105,100, 95,
90, 86, 81, 77, 73, 69, 66, 63,
59, 56, 54, 51, 48, 46, 43, 41,
39, 37, 35, 33, 32, 30, 29, 27,
26, 25, 23, 22, 21, 20, 19, 18,
17, 16, 15, 15, 14, 13, 12, 12,
11, 11, 10, 10,  9,  9,  8,  2,
240,227,216,205,195,185,175,166,
158,150,142,135,128,122,116,110,
104, 99, 94, 89, 85, 80, 76, 72,
69, 65, 62, 59, 56, 53, 50, 48,
45, 43, 41, 39, 37, 35, 33, 31,
30, 28, 27, 25, 24, 23, 22, 21,
20, 19, 18, 17, 16, 15, 14, 14,
13, 12, 12, 11, 11, 10,  9,  2
};

#ifdef _COMBINED_STATE_BIT_
extern "C" const unsigned char __declspec(align(32)) AC_next_state[512] GCC_ALIGN(32) =    
{
1,128+ 64,  2,128+  0,  3,128+  1,  4,128+  2,  5,128+  2,  6,128+  4,  7,128+  4,  8,128+  5,
9,128+  6, 10,128+  7, 11,128+  8, 12,128+  9, 13,128+  9, 14,128+ 11, 15,128+ 11, 16,128+ 12,
17,128+ 13, 18,128+ 13, 19,128+ 15, 20,128+ 15, 21,128+ 16, 22,128+ 16, 23,128+ 18, 24,128+ 18,
25,128+ 19, 26,128+ 19, 27,128+ 21, 28,128+ 21, 29,128+ 22, 30,128+ 22, 31,128+ 23, 32,128+ 24,
33,128+ 24, 34,128+ 25, 35,128+ 26, 36,128+ 26, 37,128+ 27, 38,128+ 27, 39,128+ 28, 40,128+ 29,
41,128+ 29, 42,128+ 30, 43,128+ 30, 44,128+ 30, 45,128+ 31, 46,128+ 32, 47,128+ 32, 48,128+ 33,
49,128+ 33, 50,128+ 33, 51,128+ 34, 52,128+ 34, 53,128+ 35, 54,128+ 35, 55,128+ 35, 56,128+ 36,
57,128+ 36, 58,128+ 36, 59,128+ 37, 60,128+ 37, 61,128+ 37, 62,128+ 38, 62,128+ 38, 63,128+ 63,

128+ 65,  0,128+ 66, 64,128+ 67, 65,128+ 68, 66,128+ 69, 66,128+ 70, 68,128+ 71, 68,128+ 72, 69,
128+ 73, 70,128+ 74, 71,128+ 75, 72,128+ 76, 73,128+ 77, 73,128+ 78, 75,128+ 79, 75,128+ 80, 76,
128+ 81, 77,128+ 82, 77,128+ 83, 79,128+ 84, 79,128+ 85, 80,128+ 86, 80,128+ 87, 82,128+ 88, 82,
128+ 89, 83,128+ 90, 83,128+ 91, 85,128+ 92, 85,128+ 93, 86,128+ 94, 86,128+ 95, 87,128+ 96, 88,
128+ 97, 88,128+ 98, 89,128+ 99, 90,128+100, 90,128+101, 91,128+102, 91,128+103, 92,128+104, 93,
128+105, 93,128+106, 94,128+107, 94,128+108, 94,128+109, 95,128+110, 96,128+111, 96,128+112, 97,
128+113, 97,128+114, 97,128+115, 98,128+116, 98,128+117, 99,128+118, 99,128+119, 99,128+120,100,
128+121,100,128+122,100,128+123,101,128+124,101,128+125,101,128+126,102,128+126,102,128+127,127,

1,128+ 64,  2,128+  0,  3,128+  1,  4,128+  2,  5,128+  2,  6,128+  4,  7,128+  4,  8,128+  5,
9,128+  6, 10,128+  7, 11,128+  8, 12,128+  9, 13,128+  9, 14,128+ 11, 15,128+ 11, 16,128+ 12,
17,128+ 13, 18,128+ 13, 19,128+ 15, 20,128+ 15, 21,128+ 16, 22,128+ 16, 23,128+ 18, 24,128+ 18,
25,128+ 19, 26,128+ 19, 27,128+ 21, 28,128+ 21, 29,128+ 22, 30,128+ 22, 31,128+ 23, 32,128+ 24,
33,128+ 24, 34,128+ 25, 35,128+ 26, 36,128+ 26, 37,128+ 27, 38,128+ 27, 39,128+ 28, 40,128+ 29,
41,128+ 29, 42,128+ 30, 43,128+ 30, 44,128+ 30, 45,128+ 31, 46,128+ 32, 47,128+ 32, 48,128+ 33,
49,128+ 33, 50,128+ 33, 51,128+ 34, 52,128+ 34, 53,128+ 35, 54,128+ 35, 55,128+ 35, 56,128+ 36,
57,128+ 36, 58,128+ 36, 59,128+ 37, 60,128+ 37, 61,128+ 37, 62,128+ 38, 62,128+ 38, 63,128+ 63,

128+ 65,  0,128+ 66, 64,128+ 67, 65,128+ 68, 66,128+ 69, 66,128+ 70, 68,128+ 71, 68,128+ 72, 69,
128+ 73, 70,128+ 74, 71,128+ 75, 72,128+ 76, 73,128+ 77, 73,128+ 78, 75,128+ 79, 75,128+ 80, 76,
128+ 81, 77,128+ 82, 77,128+ 83, 79,128+ 84, 79,128+ 85, 80,128+ 86, 80,128+ 87, 82,128+ 88, 82,
128+ 89, 83,128+ 90, 83,128+ 91, 85,128+ 92, 85,128+ 93, 86,128+ 94, 86,128+ 95, 87,128+ 96, 88,
128+ 97, 88,128+ 98, 89,128+ 99, 90,128+100, 90,128+101, 91,128+102, 91,128+103, 92,128+104, 93,
128+105, 93,128+106, 94,128+107, 94,128+108, 94,128+109, 95,128+110, 96,128+111, 96,128+112, 97,
128+113, 97,128+114, 97,128+115, 98,128+116, 98,128+117, 99,128+118, 99,128+119, 99,128+120,100,
128+121,100,128+122,100,128+123,101,128+124,101,128+125,101,128+126,102,128+126,102,128+127,127
};
#else
extern "C" const unsigned char __declspec(align(32)) AC_next_state[256] GCC_ALIGN(32) =    
{
1, 64,  2,  0,  3,  1,  4,  2,  5,  2,  6,  4,  7,  4,  8,  5,
9,  6, 10,  7, 11,  8, 12,  9, 13,  9, 14, 11, 15, 11, 16, 12,
17, 13, 18, 13, 19, 15, 20, 15, 21, 16, 22, 16, 23, 18, 24, 18,
25, 19, 26, 19, 27, 21, 28, 21, 29, 22, 30, 22, 31, 23, 32, 24,
33, 24, 34, 25, 35, 26, 36, 26, 37, 27, 38, 27, 39, 28, 40, 29,
41, 29, 42, 30, 43, 30, 44, 30, 45, 31, 46, 32, 47, 32, 48, 33,
49, 33, 50, 33, 51, 34, 52, 34, 53, 35, 54, 35, 55, 35, 56, 36,
57, 36, 58, 36, 59, 37, 60, 37, 61, 37, 62, 38, 62, 38, 63, 63,

65,  0, 66, 64, 67, 65, 68, 66, 69, 66, 70, 68, 71, 68, 72, 69,
73, 70, 74, 71, 75, 72, 76, 73, 77, 73, 78, 75, 79, 75, 80, 76,
81, 77, 82, 77, 83, 79, 84, 79, 85, 80, 86, 80, 87, 82, 88, 82,
89, 83, 90, 83, 91, 85, 92, 85, 93, 86, 94, 86, 95, 87, 96, 88,
97, 88, 98, 89, 99, 90,100, 90,101, 91,102, 91,103, 92,104, 93,
105, 93,106, 94,107, 94,108, 94,109, 95,110, 96,111, 96,112, 97,
113, 97,114, 97,115, 98,116, 98,117, 99,118, 99,119, 99,120,100,
121,100,122,100,123,101,124,101,125,101,126,102,126,102,127,127
};

extern "C" const unsigned char __declspec(align(32)) next_bit[256] GCC_ALIGN(32) =    
{
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,

1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0
};
#endif
*/

extern "C" const unsigned char __declspec(align(32)) times[512] GCC_ALIGN(32) = {
	0,8,7,7,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

#if !defined(CONFIG_BIARI_ENABLE_ASM) && !defined(CONFIG_BIARI_ENABLE_MMX)

unsigned int __fastcall biari_decode_final PARGS0()
{
	int delta, range;

	delta  = Ddelta;
	delta += 2;
	if(delta>=0) 
		return 1;
	range  = Drange;
	range -= 2;
	if(range<QUARTER)
	{
		range <<= 1;
		int bits_to_go = Dbits_to_go;
		unsigned int buffer;
		if(--bits_to_go<0)
		{
			unsigned int *codestrm = (unsigned int *) Dcodestrm;
			//buffer     = codestrm[0]<<24 | codestrm[1]<<16 | codestrm[2]<<8 | codestrm[3];
#ifdef _BIG_ENDIAN_
			buffer      = *codestrm;
#else
			buffer      = _bswap(*codestrm);
#endif
			codestrm   ++;
			bits_to_go += 32;
			Dcodestrm   = (unsigned char *) codestrm;
		}
		else
		{
			buffer = Dbuffer;
		}
		Dbits_to_go = bits_to_go;
		delta = (delta<<1) + (buffer>>31); // hope to use LEA
		Dbuffer = buffer<<1;
	}
	Ddelta = delta;
	Drange = range;

	return 0;
}

unsigned int __fastcall biari_decode_symbol_c PARGS1(BiContextTypePtr bi_ct)
{
	int bitsneeded, state, delta, range, lz, rLPS;

#ifdef _ARM_FIX_
	state  = *bi_ct;
#else
	state  = bi_ct->state;
#endif
	range  = Drange;
	delta  = Ddelta;
	rLPS   = total_table[state*8 + ((range-256)>>6)];
	range -= rLPS;
	lz     = (delta+rLPS)>>31;
	if(lz>=0)
		range = rLPS; // cmov ?
	delta += rLPS&lz;
	rLPS   = total_table[state*8+lz+7]; // Return value (bit)
	state  = total_table[state*8+lz+5];
#ifdef _ARM_FIX_
	*bi_ct = state;
#else
	bi_ct->state = state;
#endif

	if(range>=256)
		goto THE_END;

	//#define USE_BSR
#ifdef USE_BSR
	bitsneeded = 8-_bsr(range);
#else
	bitsneeded = times[range];
#endif

	unsigned int buffer = Dbuffer;
	range <<= bitsneeded;
	//delta     = (delta<<bitsneeded) + (buffer>>(32-bitsneeded));
	delta     = _shld(delta,buffer,bitsneeded);
	buffer <<= bitsneeded;
	int bits_to_go = Dbits_to_go - bitsneeded;
	if(bits_to_go<0)
	{
		unsigned int *codestrm = (unsigned int *) Dcodestrm;
		//buffer     = codestrm[0]<<24 | codestrm[1]<<16 | codestrm[2]<<8 | codestrm[3];
#ifdef _BIG_ENDIAN_
		buffer      = *codestrm;
#else
		buffer      = _bswap(*codestrm);
#endif
		codestrm   ++;
		delta      |= buffer>>(32+bits_to_go);
		buffer    <<= (-bits_to_go);
		bits_to_go += 32;
		Dcodestrm   = (unsigned char *) codestrm;
	}
	Dbits_to_go = bits_to_go;
	Dbuffer     = buffer;
THE_END:
	Drange      = range;
	Ddelta      = delta;
	return rLPS;
}

unsigned int __fastcall biari_decode_symbol_eq_prob_c PARGS0()
{
	int delta, range, lz;

	int bits_to_go = Dbits_to_go;
	unsigned int buffer;
	if(--bits_to_go<0)
	{
		unsigned int *codestrm = (unsigned int *) Dcodestrm;
		//buffer     = codestrm[0]<<24 | codestrm[1]<<16 | codestrm[2]<<8 | codestrm[3];
#ifdef _BIG_ENDIAN_
		buffer      = *codestrm;
#else
		buffer      = _bswap(*codestrm);
#endif
		codestrm   ++;
		bits_to_go += 32;
		Dcodestrm   = (unsigned char *) codestrm;
	}
	else
	{
		buffer = Dbuffer;
	}
	Dbits_to_go = bits_to_go;
	range = Drange;
	delta = Ddelta;
#if 1
	delta = (delta<<1) + (buffer>>31); // hope to use LEA
#else
	delta = _shld(delta,buffer,1);
#endif
	Dbuffer  = buffer<< 1; // lea

#if 0
	range += delta;
	lz = range>>31;
	Ddelta = (lz<0) ? range: delta; // cmove
#else
	lz = (delta+range)>>31;
	Ddelta = delta + (range&lz);
#endif

	return lz + 1;
}

#elif defined(CONFIG_BIARI_ENABLE_ASM)

#if !defined(_GLOBAL_IMG_)
unsigned int __declspec(naked) __fastcall biari_decode_final_asm PARGS0()
{
	// parameter 1(img): ecx

	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif        
		mov       edx, [ecx+Dep_offset+4];  // edx = Ddelta
		add       edx, 2;                   // delta += 2
		jl        LABEL1;

		mov       eax, 1;                   // return 1

		jmp       THE_END;
		//ret;

LABEL1:
		mov       eax, [ecx+Dep_offset];    // eax = Drange
		add       eax, -2;                  // range -= 2
		cmp       eax, 256;                 // compare range with QUARTER
		jl        LABEL2;

		mov       [ecx+Dep_offset], eax;    // Drange = range
		mov       [ecx+Dep_offset+4], edx;  // Ddelta = delta
		xor       eax, eax;                 // return 0

		jmp       THE_END;
		//ret;

LABEL2:
		add       eax, eax;                 // range<<=1
		mov       [ecx+Dep_offset], eax;    // Drange = range
		mov       eax, [ecx+Dep_offset+8];  // eax = Dbits_to_go
		add       eax, -1;                  // eax = Dbits_to_go-1
		jge       LABEL3;

		mov       eax, [ecx+Dep_offset+16]; // eax = Dcodestrm
		add       eax, 4;                   // eax = Dcodestrm+4
		mov       [ecx+Dep_offset+16], eax; // Dcodestrm = eax
		mov       eax, [eax-4];             // eax = *Dcodestrm
		bswap     eax;                      // little-endian to big-endian conversion
		mov       [ecx+Dep_offset+12], eax; // Dbuffer
		mov       eax, 31;                  // eax = 31
		//jmp       LABEL3;

LABEL3:
		mov       [ecx+Dep_offset+8], eax;  // Dbits_to_go = eax
		mov       eax, [ecx+Dep_offset+12]; // eax = Dbuffer
		shld      edx, eax, 1;              // delta = _shld(delta,Dbuffer,1)
		add       eax, eax;                 // Dbuffer <<= 1
		mov       [ecx+Dep_offset+12], eax; // Dbuffer
		mov       [ecx+Dep_offset+4], edx;  // Ddelta = delta
		xor       eax, eax;                 // return 0

THE_END:
		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_symbol_asm PARGS1(BiContextTypePtr bi_ct)
{
	// parameter 1(bi_ct): ecx
	// parameter 2(img): edx

	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;
#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
		mov       edx, DWORD PTR [esp+8]
#endif        

		movd      mm3, ebp;
		movd      mm2, edi;
		movd      mm1, esi;
		movd      mm0, ebx;

		mov       edi, [edx];                               // edi = range = Drange
		movzx     ebp, BYTE PTR [ecx];                      // ebp = state = bi_ct->state
		lea       esi, [edi-256];                           // esi = range with MSB removed
		mov       ebx, [edx+Dep_offset+4];                  // ebx = delta = Ddelta
		shr       esi, 6;                                   // esi = range>>6;
		movzx     eax, BYTE PTR total_table[8*ebp+esi];     // eax = rLPS
		sub       edi, eax;                                 // range -= rLPS
		lea       esi, [eax+ebx];                           // esi = temp = delta+rLPS
		test      esi, esi;                                 // To avoid partial flag stall
		cmovge    edi, eax;                                 // range = (temp>=0) ? rLPS : range
		sar       esi, 31;                                  // esi = lz = temp>>31
		and       eax, esi;                                 // eax = rLPS&lz
		add       ebx, eax;                                 // ebx = delta + rLPS&lz
		movzx     eax, BYTE PTR total_table[8*ebp+esi+5];   // eax = AC_next_state
		mov       BYTE PTR [ecx], al;                       // state = bi_ct->state = AC_next_state
		movzx     eax, BYTE PTR total_table[8*ebp+esi+7];   // eax = next_bit
		movzx     ecx, BYTE PTR times[edi];                 // ecx = bitsneeded;
		;;; Following 3 lines calculate the same as above, without the lookup - SLOWER !!!
			; bsr       ecx, edi;
		; add       ecx, -8;
		; neg       ecx;
		shl       edi, cl;                                  // range<<=bitsneeded
		mov       esi, [edx+Dep_offset+8];                  // esi = Dbits_to_go
		mov       ebp, [edx+Dep_offset+12];                 // ebp = Dbuffer
		shld      ebx, ebp, cl;                             // delta = _shld(delta,Dbuffer,bitsneeded)
		shl       ebp, cl;                                  // Dbuffer <<= bitsneeded
		sub       esi, ecx;                                 // Dbits_to_go -= bitsneeded
		jge       LABEL_A;                                  // Prob 38%

		mov       ecx, esi;                                 // ecx = Dbits_to_go (negative, but %32 is OK)
		mov       esi, [edx+Dep_offset+16];                 // esi = Dcodestrm
		add       esi, 4;                                   // 
		mov       [edx+Dep_offset+16], esi;                 // Dcodestrm += 4
		mov       esi, DWORD PTR [esi-4];                   // esi = *Dcodestrm (trick...)
		bswap     esi;                                      // little-endian to big-endian conversion
		shrd      ebp, esi, cl;                             //
		shr       esi, cl;                                  // esi = Dbuffer>>(32+Dbits_to_go)
		or        ebx, esi;                                 // delta |= Dbuffer>>(32+Dbits_to_go)
		lea       esi, [ecx+32];                            // esi = Dbits_to_go+32
		//jmp       LABEL_A;

LABEL_A:
		mov       [edx+Dep_offset+4], ebx;                  // Ddelta
		mov       [edx+Dep_offset], edi;                    // store Drange
		mov       [edx+Dep_offset+8], esi;                  // Dbits_to_go
		mov       [edx+Dep_offset+12], ebp;                 // Dbuffer

		movd      ebx, mm0;
		movd      esi, mm1;
		movd      edi, mm2;
		movd      ebp, mm3;

		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_symbol_eq_prob_asm PARGS0()
{
	// parameter 1(img): ecx

	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif        

		mov       edx, [ecx+Dep_offset+8];  // edx = Dbits_to_go
		add       edx, -1;                  // edx = Dbits_to_go-1
		jge       LABEL_POS;

		mov       eax, [ecx+Dep_offset+16]; // eax = Dcodestrm
		add       eax, 4;                   // eax = Dcodestrm+4
		mov       [ecx+Dep_offset+16], eax; // Dcodestrm = eax
		mov       eax, [eax-4];             // eax = *Dcodestrm (trick)
		bswap     eax;                      // little-endian to big-endian conversion
		add       edx, 32;                  // edx = 31
		jmp       LABEL_ELSE;

LABEL_POS:
		mov       eax, [ecx+Dep_offset+12]; // eax = Dbuffer
LABEL_ELSE:
		mov       [ecx+Dep_offset+8], edx;  // Dbits_to_go = edx
		mov       edx, [ecx+Dep_offset+4];  // edx = Ddelta
		shld      edx, eax, 1;              // edx = _shld(delta, Dbuffer, 1)
		add       eax, eax;                 // Dbuffer<<=1;

		mov       [ecx+Dep_offset+12], eax; // Dbuffer
		mov       eax, [ecx+Dep_offset];    // eax = Drange;
		add       eax, edx;                 // range += delta
		cmovl     edx, eax;                 // delta = (delta<0) ? range : delta
		sar       eax, 31;                  // range>>=31 (lz)
		mov       [ecx+Dep_offset+4], edx;  // Ddelta
		add       eax, 1;                   // return (lz +1)

		ret;
	}
}

#else

unsigned int __declspec(naked) __fastcall biari_decode_final_asm PARGS0()
{
	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

		mov       edx, Ddelta;            // edx = Ddelta
		add       edx, 2;                 // delta += 2
		jl        LABEL1;

		mov       eax, 1;                 // return 1

		jmp       THE_END;
		//ret;

LABEL1:
		mov       eax, Drange;            // eax = Drange
		add       eax, -2;                // range -= 2
		cmp       eax, 256;               // compare range with QUARTER
		jl        LABEL2;

		mov       Drange, eax;            // Drange = range
		mov       Ddelta, edx;            // Ddelta = delta
		xor       eax, eax;               // return 0

		jmp       THE_END;
		//ret;

LABEL2:
		add       eax, eax;               // range<<=1
		mov       Drange, eax;            // Drange = range
		mov       ecx, Dbits_to_go;       // ecx = Dbits_to_go
		add       ecx, -1;                // ecx = Dbits_to_go-1
		jge       LABEL3;

		mov       eax, Dcodestrm;         // eax = Dcodestrm
		add       eax, 4;                 // eax = Dcodestrm+4
		mov       Dcodestrm, eax;         // Dcodestrm = eax
		mov       eax, DWORD PTR [eax-4]; // eax = *Dcodestrm (trick)
		bswap     eax;                    // little-endian to big-endian conversion
		add       ecx, 32;                // ecx = 31
		jmp       LABEL4;

LABEL3:
		mov       eax, Dbuffer;           // eax = Dbuffer
LABEL4:
		mov       Dbits_to_go, ecx;       // Dbits_to_go = ecx
		shld      edx, eax, 1;            // delta = _shld(delta,Dbuffer,1)
		add       eax, eax;               // Dbuffer <<= 1
		mov       Dbuffer, eax;           // Dbuffer
		mov       Ddelta, edx;            // Ddelta = delta
		xor       eax, eax;               // return 0

THE_END:
		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_symbol_asm PARGS1(BiContextTypePtr bi_ct)
{
	// parameter 1(bi_ct): ecx
	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif        
		//int       3;
		movd      mm2, edi;
		movd      mm1, esi;
		movd      mm0, ebx;

		mov       edi, Drange;                              // edi = range = Drange
		movzx     edx, BYTE PTR [ecx];                      // edx = state = bi_ct->state
		lea       esi, [edi-256];                           // esi = range, removing MSB !
		mov       ebx, Ddelta;                              // ebx = delta = Ddelta
		shr       esi, 6;                                   // esi = range>>6 (0..3)
		movzx     eax, BYTE PTR total_table[8*edx+esi];     // eax = rLPS
		sub       edi, eax;                                 // range -= rLPS
#if 0
		mov       esi, ebx;                                 // esi = delta
		add       esi, eax;                                 // esi = temp = delta+rLPS
#else
		lea       esi, [eax+ebx];                           // esi = temp = delta+rLPS
		test      esi, esi;                                 // To avoid partial flag stall
#endif
		cmovge    edi, eax;                                 // range = (temp>=0) ? rLPS : range
#if 0
		cmovl     ebx, esi;                                 // ebx = (temp<0) ? delta + rLPS : delta
		sar       esi, 31;                                  // esi = lz = temp>>31
#else
		sar       esi, 31;                                  // esi = lz = temp>>31
		and       eax, esi;                                 // eax = rLPS&lz
		add       ebx, eax;                                 // ebx = delta + rLPS&lz
#endif
		movzx     eax, BYTE PTR total_table[8*edx+esi+5];   // eax = AC_next_state
		mov       BYTE PTR [ecx], al;                       // state = bi_ct->state = AC_next_state
		movzx     eax, BYTE PTR total_table[8*edx+esi+7];   // eax = next_bit
		movzx     ecx, BYTE PTR times[edi];                 // ecx = bitsneeded;
		;;; Following 3 lines calculate the same as above, without the lookup - SLOWER !!!
			; bsr       ecx, edi;
		; add       ecx, -8;
		; neg       ecx;
		shl       edi, cl;                                  // range<<=bitsneeded
		mov       esi, Dbits_to_go;                         // esi = Dbits_to_go
		mov       edx, Dbuffer;                             // edx = Dbuffer
		shld      ebx, edx, cl;                             // delta = _shld(delta,Dbuffer,bitsneeded)
		shl       edx, cl;                                  // Dbuffer <<= bitsneeded
		sub       esi, ecx;                                 // Dbits_to_go -= bitsneeded
		jge       LABEL_A;                                  // Prob 38%

#if 1
		mov       ecx, esi;                                 // ecx = Dbits_to_go (negative, but %32 is OK)
		mov       esi, Dcodestrm;                           // esi = Dcodestrm
		add       esi, 4;                                   // 
		mov       Dcodestrm, esi;                           // Dcodestrm += 4
		mov       esi, DWORD PTR [esi-4];                   // esi = *Dcodestrm (trick...)
		bswap     esi;                                      // little-endian to big-endian conversion
		shrd      edx, esi, cl;                             // 
		shr       esi, cl;                                  // esi = Dbuffer>>(32+Dbits_to_go)
		or        ebx, esi;                                 // delta |= Dbuffer>>(32+Dbits_to_go)
		lea       esi, [ecx+32];                            // esi = Dbits_to_go+32
#else
		mov       edx, Dcodestrm;                           // edx = Dcodestrm
		add       edx, 4;                                   // 
		mov       Dcodestrm, edx;                           // Dcodestrm += 4
		mov       edx, DWORD PTR [edx-4];                   // edx = *Dcodestrm (trick...)
		bswap     edx;                                      // little-endian to big-endian conversion
#if 0
		lea       ecx, [esi+32];                            // ecx = Dbits_to_go+32
		mov       esi, edx;                                 // esi = Dbuffer
		shr       esi, cl;                                  // esi = Dbuffer>>(32+Dbits_to_go)
		or        ebx, esi;                                 // delta |= Dbuffer>>(32+Dbits_to_go)
		mov       esi, ecx;                                 // esi = Dbits_to_go+32
		add       ecx, -32;                                 // ecx = Dbits_to_go
#else
		mov       ecx, esi;                                 // ecx = Dbits_to_go (negative, but %32 is OK)
		mov       esi, edx;                                 // esi = Dbuffer
		shr       esi, cl;                                  // esi = Dbuffer>>(32+Dbits_to_go)
		or        ebx, esi;                                 // delta |= Dbuffer>>(32+Dbits_to_go)
		lea       esi, [ecx+32];                            // esi = Dbits_to_go+32
#endif
		neg       ecx;                                      // ecx = -Dbits_to_go
		shl       edx, cl;                                  // Dbuffer <<= (-Dbits_to_go)
#endif
		//jmp       LABEL_A;

LABEL_A:
		mov       Ddelta, ebx;                              // Ddelta
		mov       Drange, edi;                              // store Drange
		mov       Dbits_to_go, esi;                         // Dbits_to_go
		mov       Dbuffer, edx;                             // Dbuffer

		movd      ebx, mm0;
		movd      esi, mm1;
		movd      edi, mm2;
		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_symbol_eq_prob_asm PARGS0()
{

	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

		mov       edx, Dbits_to_go;           // edx = Dbits_to_go
		add       edx, -1;                    // edx = Dbits_to_go-1
		jge       LABEL_POS;

		mov       eax, Dcodestrm;             // eax = Dcodestrm
		add       eax, 4;                     // 
		mov       Dcodestrm, eax;             // Dcodestrm += 4
		mov       eax, DWORD PTR [eax-4];     // eax = *Dcodestrm (trick)
		bswap     eax;                        // little-endian to big-endian conversion
		add       edx, 32;                    // edx = 31
		jmp       LABEL_ELSE;

LABEL_POS:
		mov       eax, Dbuffer;               // eax = Dbuffer
LABEL_ELSE:
		mov       Dbits_to_go, edx;           // Dbits_to_go = edx
		mov       edx, Ddelta;                // edx = Ddelta
		shld      edx, eax, 1;                // edx = _shld(delta, Dbuffer, 1)
		; Following 3 lines are equivalent to the 1 above
			; mov       ecx, eax;
		; shr       ecx, 31;
		; lea       edx, [2*edx+ecx];
		add       eax, eax;                   // Dbuffer<<=1;

		mov       Dbuffer, eax;               // Dbuffer
		mov       ecx, Drange;                // ecx = Drange;
		lea       eax, [ecx+edx];             // eax = (range+delta)
		sar       eax, 31;                    // lz = (range+delta)>>=31
		and       ecx, eax;                   // range&lz
		add       edx, ecx;                   // delta = delta + (range&lz)
		add       eax, 1;                     // return (lz +1)
		mov       Ddelta, edx;                // Ddelta
		; Following 6 lines are equivalent to the above 7
			; mov       eax, Drange;                // eax = Drange
		; add       eax, edx;                   // range += delta
		; cmovl     edx, eax;                   // delta = (delta<0) ? range : delta
		; sar       eax, 31;                    // range>>=31 (lz)
		; mov       Ddelta, edx;                // Ddelta
		; add       eax, 1;                     // return (lz+1)

		ret;
	}
}

#endif // _GLOBAL_IMG_ or not

#elif defined(CONFIG_BIARI_ENABLE_MMX)

#if CONFIG_BIARI_ENABLE_MMX==1

void __declspec(naked) __fastcall store_dep_mmx PARGS0()
{
	// parameter 1(img): ecx
	__asm
	{
		ALIGN CODE_ALIGNMENT_PROPERTY;
#ifdef _GLOBAL_IMG_
		movd  mm7, Drange;       // Drange
		movd  mm6, Ddelta;       // Ddelta
		movd  mm5, Dbits_to_go;  // Dbits_to_go
		movd  mm4, Dbuffer;      // Dbuffer
		movd  mm3, Dcodestrm;    // Dcodestrm
#else
#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif

		movd  mm7, [ecx+Dep_offset];    // Drange
		movd  mm6, [ecx+Dep_offset+4];  // Ddelta
		movd  mm5, [ecx+Dep_offset+8];  // Dbits_to_go
		movd  mm4, [ecx+Dep_offset+12]; // Dbuffer
		movd  mm3, [ecx+Dep_offset+16]; // Dcodestrm
#endif
		ret;
	}
}

void __declspec(naked) __fastcall load_dep_mmx PARGS0()
{
	// parameter 1(img): ecx

	__asm
	{
		ALIGN CODE_ALIGNMENT_PROPERTY;
#ifdef _GLOBAL_IMG_
		movd  Drange, mm7;       // Drange
		movd  Ddelta, mm6;       // Ddelta
		movd  Dbits_to_go,  mm5; // Dbits_to_go
		movd  Dbuffer, mm4;      // Dbuffer
		movd  Dcodestrm, mm3;    // Dcodestrm
#else
#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif        

		movd  [ecx+Dep_offset], mm7;    // Drange
		movd  [ecx+Dep_offset+4], mm6;  // Ddelta
		movd  [ecx+Dep_offset+8],  mm5; // Dbits_to_go
		movd  [ecx+Dep_offset+12], mm4; // Dbuffer
		movd  [ecx+Dep_offset+16], mm3; // Dcodestrm
#endif
		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_final_mmx ()
{
	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

		movd      edx, mm6;               // edx = Ddelta
		add       edx, 2;                 // delta += 2
		jl        LABEL1;

		mov       eax, 1;                 // return 1

		jmp       THE_END;
		//ret;

LABEL1:
		movd      eax, mm7;               // eax = Drange
		add       eax, -2;                // range -= 2
		cmp       eax, 256;               // compare range with QUARTER
		jl        LABEL2;

		movd      mm7, eax;               // Drange = range
		movd      mm6, edx;               // Ddelta = delta
		xor       eax, eax;               // return 0

		jmp       THE_END;
		//ret;

LABEL2:
		add       eax, eax;               // range<<=1
		movd      mm7, eax;               // Drange = range
		movd      ecx, mm5;               // ecx = Dbits_to_go
		add       ecx, -1;                // ecx = Dbits_to_go-1
		jge       LABEL3;

		movd      eax, mm3;               // eax = Dcodestrm
		add       eax, 4;                 // eax = Dcodestrm+4
		movd      mm3, eax;               // Dcodestrm = eax
		mov       eax, DWORD PTR [eax-4]; // eax = *Dcodestrm (trick)
		bswap     eax;                    // little-endian to big-endian conversion
		add       ecx, 32;                // ecx = 31
		jmp       LABEL4;

LABEL3:
		movd      eax, mm4;               // eax = Dbuffer
LABEL4:
		movd      mm5, ecx;               // Dbits_to_go = ecx
		shld      edx, eax, 1;            // delta = _shld(delta,Dbuffer,1)
		add       eax, eax;               // Dbuffer <<= 1
		movd      mm4, eax;               // Dbuffer
		movd      mm6, edx;               // Ddelta = delta
		xor       eax, eax;               // return 0

THE_END:
		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_symbol_mmx (BiContextTypePtr bi_ct)
{
	// parameter 1(bi_ct): ecx

	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif
		movd      mm2, edi;
		movd      mm1, esi;
		movd      mm0, ebx;

		movzx     edx, BYTE PTR [ecx];                      // edx = state = bi_ct->state
		movd      edi, mm7;                                 // edi = range = Drange
		lea       esi, [edi-256];                           // esi = range with MSB removed
		movd      ebx, mm6;                                 // ebx = delta = Ddelta
		shr       esi, 6;                                   // esi = range>>6
		movzx     eax, BYTE PTR total_table[8*edx+esi];     // eax = rLPS
		sub       edi, eax;                                 // range -= rLPS
		lea       esi, [eax+ebx];                           // esi = delta
		test      esi, esi;                                 // To avoid partial flag stall
		cmovge    edi, eax;                                 // range = (temp>=0) ? rLPS : range
		sar       esi, 31;                                  // esi = lz = temp>>31
		and       eax, esi;                                 // eax = rLPS&lz
		add       ebx, eax;                                 // ebx = delta + rLPS&lz
		movzx     eax, BYTE PTR total_table[8*edx+esi+5];   // eax = AC_next_state
		mov       BYTE PTR [ecx], al;                       // state = bi_ct->state = AC_next_state
		movzx     eax, BYTE PTR total_table[8*edx+esi+7];   // eax = next_bit
		movzx     ecx, BYTE PTR times[edi];                 // ecx = bitsneeded;
		;;; Following 3 lines calculate the same as above, without the lookup - SLOWER !!!
			; bsr       ecx, edi;
		; add       ecx, -8;
		; neg       ecx;
		shl       edi, cl;                                  // range<<=bitsneeded
		movd      esi, mm5;                                 // esi = Dbits_to_go
		movd      edx, mm4;                                 // edx = Dbuffer
		shld      ebx, edx, cl;                             // delta = _shld(delta,Dbuffer,bitsneeded)
		shl       edx, cl;                                  // Dbuffer <<= bitsneeded
		sub       esi, ecx;                                 // Dbits_to_go -= bitsneeded
		jge       LABEL_A;                                  // Prob 38%

		mov       ecx, esi;                                 // ecx = Dbits_to_go (negative, but %32 is OK)
		movd      esi, mm3;                                 // esi = Dcodestrm
		add       esi, 4;                                   // 
		movd      mm3, esi;                                 // Dcodestrm += 4
		mov       esi, DWORD PTR [esi-4];                   // esi = *Dcodestrm (trick...)
		bswap     esi;                                      // little-endian to big-endian conversion
		shrd      edx, esi, cl;                             // 
		shr       esi, cl;                                  // esi = Dbuffer>>(32+Dbits_to_go)
		or        ebx, esi;                                 // delta |= Dbuffer>>(32+Dbits_to_go)
		lea       esi, [ecx+32];                            // esi = Dbits_to_go+32
		//jmp       LABEL_A;

LABEL_A:
		movd      mm6, ebx;                                 // Ddelta
		movd      mm7, edi;                                 // store Drange
		movd      mm5, esi;                                 // Dbits_to_go
		movd      mm4, edx;                                 // Dbuffer

		movd      ebx, mm0;
		movd      esi, mm1;
		movd      edi, mm2;

		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_symbol_eq_prob_mmx ()
{

	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

		movd      edx, mm5;         // edx = Dbits_to_go
		add       edx, -1;          // edx = Dbits_to_go-1
		jge       LABEL_POS;

		movd      eax, mm3;         // eax = Dcodestrm
		add       eax, 4;           // eax = Dcodestrm+4
		movd      mm3, eax;         // Dcodestrm = eax
		mov       eax, [eax-4];     // eax = *Dcodestrm (trick)
		bswap     eax;              // little-endian to big-endian conversion
		add       edx, 32;          // edx = 31
		jmp       LABEL_ELSE;

LABEL_POS:
		movd      eax, mm4;         // eax = Dbuffer
LABEL_ELSE:
		movd      mm5, edx;         // Dbits_to_go = edx
		movd      edx, mm6;         // edx = Ddelta
		shld      edx, eax, 1;      // edx = _shld(delta, Dbuffer, 1)
		; Following 3 lines are equivalent to the 1 above
			; mov       ecx, eax;
		; shr       ecx, 31;
		; lea       edx, [2*edx+ecx];
		add       eax, eax;        // Dbuffer<<=1;

		movd      mm4, eax;         // Dbuffer
		movd      ecx, mm7;         // ecx = Drange;
		lea       eax, [ecx+edx];   // eax = (range+delta)
		sar       eax, 31;          // lz = (range+delta)>>=31
		and       ecx, eax;         // range&lz
		add       edx, ecx;         // delta = delta + (range&lz)
		add       eax, 1;           // return (lz +1)
		movd      mm6, edx;         // Ddelta
		; Following 6 lines are equivalent to the above 7
			; movd      eax, mm7;         // eax = Drange
		; add       eax, edx;         // range += delta
		; cmovl     edx, eax;         // delta = (delta<0) ? range : delta
		; sar       eax, 31;          // range>>=31 (lz)
		; movd      mm6, edx;         // Ddelta
		; add       eax, 1;           // return (lz+1)

		ret;
	}
}

#else // CONFIG_BIARI_ENABLE_MMX!=1
// This is an alternative implementation of the 3 biari_decode_symbol functions
// that uses extensively MMX instructions.
void __declspec(naked) __fastcall store_dep_mmx PARGS0()
{
	// parameter 1(img): ecx

	__asm
	{
		ALIGN   CODE_ALIGNMENT_PROPERTY;
#ifdef _GLOBAL_IMG_
		movd    mm0, Drange;      // Drange
		movd    mm1, Dbuffer;     // Dbuffer
		pinsrw  mm1, Ddelta, 2;   // Ddelta
		movd    mm2, Dbits_to_go; // Dbits_to_go
		movd    mm3, Dcodestrm;   // Dcodestrm
#else
#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif
		movd    mm0, DWORD PTR [ecx+Dep_offset];     // Drange
		movd    mm1, DWORD PTR [ecx+Dep_offset+12];  // Dbuffer
		pinsrw  mm1, WORD PTR [ecx+Dep_offset+4], 2; // Ddelta
		movd    mm2, [ecx+Dep_offset+8];             // Dbits_to_go
		movd    mm3, [ecx+Dep_offset+16];            // Dcodestrm
#endif
		ret;
	}
}

void __declspec(naked) __fastcall load_dep_mmx PARGS0()
{
	// parameter 1(img): ecx

	__asm
	{
		ALIGN   CODE_ALIGNMENT_PROPERTY;
#ifdef _GLOBAL_IMG_
		movd    Drange, mm0;      // Drange
		movd    Dbuffer, mm1;     // Dbuffer
		pextrw  eax, mm1, 2;
		mov     WORD PTR Ddelta, ax;       // Ddelta
		movd    Dbits_to_go, mm2; // Dbits_to_go
		movd    Dcodestrm, mm3;   // Dcodestrm
#else
#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif        
		movd    DWORD PTR [ecx+Dep_offset], mm0;     // Drange
		movd    DWORD PTR [ecx+Dep_offset+12], mm1;  // Dbuffer
		pextrw  eax, mm1, 2;
		mov     WORD PTR [ecx+Dep_offset+4], ax;     // Ddelta
		movd    [ecx+Dep_offset+8], mm2;             // Dbits_to_go
		movd    [ecx+Dep_offset+16], mm3;            // Dcodestrm
#endif
		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_final_mmx ()
{

	__asm
	{    
		ALIGN     CODE_ALIGNMENT_PROPERTY;

		pextrw    edx, mm1, 2;            // edx = Ddelta
		add       dx, 2;                  // delta += 2
		jl        LABEL1;

		mov       eax, 1;                 // return 1

		jmp       THE_END;
		//ret;

LABEL1:
		movd      eax, mm0;               // eax = Drange
		add       eax, -2;                // range -= 2
		movd      mm0, eax;               // Drange = range
		pinsrw    mm1, edx, 2;            // Ddelta = delta
		cmp       eax, 256;               // compare range with QUARTER
		jl        LABEL2;

		xor       eax, eax;               // return 0

		jmp       THE_END;
		//ret;

LABEL2:
		pslld     mm0, 1;                 // Drange<<=1
		movd      ecx, mm2;               // ecx = Dbits_to_go
		add       ecx, -1;                // ecx = Dbits_to_go-1
		jge       LABEL3;

		movd      eax, mm3;               // eax = Dcodestrm
		add       eax, 4;                 // eax = Dcodestrm+4
		movd      mm3, eax;               // Dcodestrm = eax
		mov       eax, DWORD PTR [eax-4]; // eax = *Dcodestrm (trick)
		bswap     eax;                    // little-endian to big-endian conversion
		movd      mm4, eax;               // Dbuffer
		add       ecx, 32;                // ecx = 31
		por       mm1, mm4;
		//jmp       LABEL3;

LABEL3:
		movd      mm2, ecx;               // Dbits_to_go = ecx
		psllq     mm1, 1;                 // delta = _shld(delta,Dbuffer,1), Dbuffer <<= 1
		xor       eax, eax;               // return 0

THE_END:
		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_symbol_mmx (BiContextTypePtr bi_ct)
{
	// parameter 1(bi_ct): ecx

	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

#ifdef __linux__
		mov       ecx, DWORD PTR [esp+4]
#endif        
		movd      mm7, ebx;
		movd      mm6, esi;

		movd      esi, mm0;                                 // esi = range = Drange
		movzx     edx, BYTE PTR [ecx];                      // edx = state = bi_ct->state
		lea       ebx, [esi-256];                           // ebx = range with MSB removed
		shr       ebx, 6;                                   // ebx = range>>6
		movzx     eax, BYTE PTR total_table[8*edx+ebx];     // eax = rLPS
		pextrw    ebx, mm1, 2;                              // ebx = delta = Ddelta
		movsx     edx, bx;                                  // edx = delta (orig)
		sub       esi, eax;                                 // range -= rLPS
		add       edx, eax;                                 // delta += rLPS
		cmovge    esi, eax;                                 // range = (delta<0) ? range : rLPS
		mov       eax, edx;                                 // eax = delta
		cmovge    edx, ebx;                                 // delta = (delta<0) ? delta (new) : delta (orig)
		movd      mm0, esi;                                 // Drange = range
		pinsrw    mm1, edx, 2;                              // Ddelta = delta
		sar       eax, 31;                                  // eax = lz = delta>>31
		movzx     edx, BYTE PTR [ecx];                      // edx = state = bi_ct->state
		movzx     ebx, BYTE PTR total_table[8*edx+eax+5];   // ebx = AC_next_state
		mov       BYTE PTR [ecx], bl;                       // bi_ct->state = AC_next_state
		movzx     eax, BYTE PTR total_table[8*edx+eax+7];   // eax = next_bit
		movzx     ecx, BYTE PTR times[esi];                 // ecx = bitsneeded;
		;;; Following 3 lines calculate the same as above, without the lookup - SLOWER !!!
			; bsr       ecx, esi;
		; add       ecx, -8;
		; neg       ecx;
		movd      mm4, ecx;                                 // mm5 = bitsneeded
		pslld     mm0, mm4;                                 // range<<=bitsneeded;
		psllq     mm1, mm4;                                 // delta = _shld(delta,Dbuffer,bitsneeded), Dbuffer <<= bitsneeded
		psubd     mm2, mm4;                                 // Dbits_to_go -= bitsneeded
		movd      ecx, mm2;
		test      ecx, ecx;
		jge       LABEL_A;                                  // Prob 38%

		lea       esi, [ecx+32];                            // esi = Dbits_to_go+32
		movd      edx, mm3;                                 // edx = Dcodestrm
		add       edx, 4;                                   // 
		movd      mm3, edx;                                 // Dcodestrm += 4
		mov       edx, DWORD PTR [edx-4];                   // edx = *Dcodestrm (trick)
		neg       ecx;
		bswap     edx;                                      // little-endian to big-endian conversion
		movd      mm4, ecx;                                 // mm4 = (-Dbits_to_go), so positive
		movd      mm5, edx;                                 // mm5 = buffer
		psrld     mm1, mm4;
		por       mm1, mm5;
		psllq     mm1, mm4;
		movd      mm2, esi;                                 // Dbits_to_go+=32
		//jmp       LABEL_A;

LABEL_A:
		movd      esi, mm6;
		movd      ebx, mm7;

		ret;
	}
}

unsigned int __declspec(naked) __fastcall biari_decode_symbol_eq_prob_mmx ()
{

	__asm
	{
		ALIGN     CODE_ALIGNMENT_PROPERTY;

		movd      edx, mm2;         // edx = Dbits_to_go
		add       edx, -1;          // edx = Dbits_to_go-1
		jge       LABEL_POS;

		movd      ecx, mm3;         // ecx = Dcodestrm
		add       ecx, 4;           // ecx = Dcodestrm+4
		movd      mm3, ecx;         // Dcodestrm = ecx
		mov       ecx, [ecx-4];     // ecx = *Dcodestrm (trick)
		bswap     ecx;              // little-endian to big-endian conversion
		movd      mm4, ecx;
		add       edx, 32;          // edx = 31
		por       mm1, mm4;         // Pack delta with buffer
		//jmp       LABEL_POS;

LABEL_POS:
		movd      mm2, edx;         // Dbits_to_go = edx
		psllq     mm1, 1;
		pshufw    mm5, mm0, 79;     // mm5 = (0, range, 0, 0)
		paddsw    mm1, mm5;
		pextrw    eax, mm1, 2;      // eax = Ddelta
		sar       ax, 15;           // range>>=31 (lz)
		movd      mm4, eax;         // mm4 = (0, 0, 0, lz)
		psllq     mm4, 32;          // mm4 = (0, lz, 0, 0)
		pandn     mm4, mm5;         // mm4 = (0,~lz&range,0,0)
		add       ax, 1;            // return (lz +1)
		psubsw    mm1, mm4;

		ret;
	}
}

#endif // CONFIG_BIARI_ENABLE_MMX == 1 or == 2

#endif // CONFIG_BIARI_ENABLE_ASM or _MMX or none
