
/*!
************************************************************************
* \file vlc.c
*
* \brief
*    VLC support functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Inge Lille-Langøy               <inge.lille-langoy@telenor.com>
*    - Detlev Marpe                    <marpe@hhi.de>
*    - Gabi Blaettermann               <blaetter@hhi.de>
************************************************************************
*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "vlc.h"
#include "elements.h"
#include "clipping.h"

#ifdef _GLOBAL_IMG_
#define DPP img.g_dep.
#else
#define DPP img->g_dep.
#endif

#define Dcodestrm		DPP Dcodestrm
#define Dbits_to_go		DPP	Dbits_to_go
#define Dbuffer			DPP Dbuffer
#define Dstrmlength     DPP Dstrmlength
#define Dbasestrm       DPP Dbasestrm

//code,length
#define _C0L(val,len)		(val<<4)|(len) //byte
#define _C1L(val,len)		(val<<8)|(len)
#define _C2L(val1,val2,len)	(val1<<13)|(val2<<8)|(len)


const int LEVEL_PREFIX_TAB[16] = 
{
	4,
	3,
	2,2,
	1,1,1,1,
	0,0,0,0,0,0,0,0
};

// A little trick to avoid those horrible #if TRACE all over the source code
extern void tracebits(const char *trace_str,  int len,  int info,int value1);


// Note that all NA values are filled with 0

//! for the linfo_levrun_inter routine
const byte NTAB1[4][8][2] =
{
	{{1,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
	{{1,1},{1,2},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
	{{2,0},{1,3},{1,4},{1,5},{0,0},{0,0},{0,0},{0,0}},
	{{3,0},{2,1},{2,2},{1,6},{1,7},{1,8},{1,9},{4,0}},
};
const byte LEVRUN1[16]=
{
	4,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0,
};


const byte NTAB2[4][8][2] =
{
	{{1,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
	{{1,1},{2,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
	{{1,2},{3,0},{4,0},{5,0},{0,0},{0,0},{0,0},{0,0}},
	{{1,3},{1,4},{2,1},{3,1},{6,0},{7,0},{8,0},{9,0}},
};

//! for the linfo_levrun__c2x2 routine
const byte LEVRUN3[4] =
{
	2,1,0,0
};
const byte NTAB3[2][2][2] =
{
	{{1,0},{0,0}},
	{{2,0},{1,1}},
};

//! gives CBP value from codeword number, both for intra and inter
extern const unsigned char _NCBP[2][48][2]=
{
	{  // 0      1        2       3       4       5       6       7       8       9      10      11
		{15, 0},{ 0, 1},{ 7, 2},{11, 4},{13, 8},{14, 3},{ 3, 5},{ 5,10},{10,12},{12,15},{ 1, 7},{ 2,11},
		{ 4,13},{ 8,14},{ 6, 6},{ 9, 9},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},
		{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},
		{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}
	},
	{
		{47, 0},{31,16},{15, 1},{ 0, 2},{23, 4},{27, 8},{29,32},{30, 3},{ 7, 5},{11,10},{13,12},{14,15},
		{39,47},{43, 7},{45,11},{46,13},{16,14},{ 3, 6},{ 5, 9},{10,31},{12,35},{19,37},{21,42},{26,44},
		{28,33},{35,34},{37,36},{42,40},{44,39},{ 1,43},{ 2,45},{ 4,46},{ 8,17},{17,18},{18,20},{20,24},
		{24,19},{ 6,21},{ 9,26},{22,28},{25,23},{32,27},{33,29},{34,30},{36,22},{40,25},{38,38},{41,41}
	}
};

/*! 
*************************************************************************************
* \brief
*    ue_v, reads an ue(v) syntax element
*
* \param ImageParameters *
*    in order to access the DecodingEnvironment to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
int ue_v_no_string PARGS0()
{
	assert (Dbasestrm != NULL);
	return readSyntaxElement_VLC_ue ARGS0();
}


/*! 
*************************************************************************************
* \brief
*    se_v, reads an se(v) syntax element
*
* \param ImageParameters *
*    in order to access the stream to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
int se_v_no_string PARGS0()
{
	assert (Dbasestrm != NULL);
	return readSyntaxElement_VLC_se ARGS0();
}


/*! 
*************************************************************************************
* \brief
*    u_v, reads an u(v) syntax element
*
* \param LenInBits
*    length of the syntax element
*
* \param ImageParameters *
*    in order to access the stream to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
int u_v_no_string PARGS1(int LenInBits)
{
	assert (Dbasestrm != NULL);
	return readSyntaxElement_FLC ARGS1(LenInBits);
};


/*! 
*************************************************************************************
* \brief
*    u_1, reads an u(1) syntax element
*
* \param ImageParameters *
*    in order to access the stream to be read from
*
* \return
*    the value of the coded syntax element
*
*************************************************************************************
*/
int u_1_no_string PARGS0()
{
	return u_v_no_string ARGS1(1);
}



/*!
************************************************************************
* \brief
*    mapping rule for ue(v) syntax elements
* \par Input:
*    lenght and info
* \par Output:
*    number in the code table
************************************************************************
*/
static inline int __fastcall linfo_ue(int len, int info)
{
	return (1<<len)+info-1;
	//*value1 = (int)pow(2,(len/2))+info-1; // *value1 = (int)(2<<(len>>1))+info-1;
}

/*!
************************************************************************
* \brief
*    mapping rule for se(v) syntax elements
* \par Input:
*    lenght and info
* \par Output:
*    signed mvd
************************************************************************
*/
static inline int __fastcall linfo_se(int len, int info)
{
	int n;
	int sign;
	n = (1<<len)+info;  //n = (int)pow(2,(len/2))+info-1;
	sign = -(n & 1);    //lsb is sign bit
	return (n>>1)-(sign&(n-1));
}


/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    cbp (intra)
************************************************************************
*/
static inline int __fastcall linfo_cbp_intra PARGS2(int len, int info)
{
	int cbp_idx;

	cbp_idx = linfo_ue(len,info);
	return NCBP[2*cbp_idx+0];
}

/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    cbp (inter)
************************************************************************
*/
static inline int __fastcall linfo_cbp_inter PARGS2(int len, int info)
{
	int cbp_idx;

	cbp_idx = linfo_ue(len,info);
	return NCBP[2*cbp_idx+1];
}

/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    level, run
************************************************************************
*/
static inline void __fastcall linfo_levrun_inter(int len, int info, int *level, int *irun)
{
	int l2;
	int inf;
	if (len<=4)
	{
		l2=max(0,len-1);
		inf=info/2;
		*level=NTAB1[l2][inf][0];
		*irun=NTAB1[l2][inf][1];
		if ((info&0x01)==1)
			*level=-*level;                   // make sign
	}
	else                                  // if len > 9, skip using the array
	{
		*irun=(info&0x1e)>>1;
		*level = LEVRUN1[*irun] + info/32 + (1L<<(len-5));
		//*level = LEVRUN1[*irun] + info/32 + (int)pow(2,len/2 - 5);
		if ((info&0x01)==1)
			*level=-*level;
	}
	if (len == 0) // EOB
		*level = 0;
}


/*!
************************************************************************
* \par Input:
*    length and info
* \par Output:
*    level, run
************************************************************************
*/
static inline void __fastcall linfo_levrun_c2x2(int len, int info, int *level, int *irun)
{
	int l2;
	int inf;

	if (len<=2)
	{
		l2=max(0,len-1);
		inf=info/2;
		*level=NTAB3[l2][inf][0];
		*irun=NTAB3[l2][inf][1];
		if ((info&0x01)==1)
			*level=-*level;                 // make sign
	}
	else                                  // if len > 5, skip using the array
	{
		*irun=(info&0x06)>>1;
		*level = LEVRUN3[*irun] + info/8 + (1<<(len-3));
		//*level = LEVRUN3[*irun] + info/8 + (int)pow(2,len/2 - 3);
		if ((info&0x01)==1)
			*level=-*level;
	}
	if (len == 0) // EOB
		*level = 0;
}

// IOK: _RIGHT_JUSTIFY_ is faster
#define _RIGHT_JUSTIFY_

#ifdef _RIGHT_JUSTIFY_
static unsigned int __declspec(align(32)) masks[33] GCC_ALIGN(32) =
{ 0x00000000, 0x00000001, 0x00000003, 0x00000007, 
0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F, 
0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF, 
0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF, 
0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 
0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 
0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 
0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF };
#endif // _RIGHT_JUSTIFY_

/*!
************************************************************************
* \brief
*  Reads bits from the DecodingEnvironment buffer and advances bit pointer
*
* \param numbits
*    number of bits to read
* \param img
*    pointer to decoding engine state
*
************************************************************************
*/
#ifdef _RIGHT_JUSTIFY_
static inline unsigned int __fastcall H264GetBits PARGS1(int numbits)
{
	unsigned int val;

	Dbits_to_go -= numbits;
	if(Dbits_to_go<0)
	{
		val          = Dbuffer<<(-Dbits_to_go);
#ifdef _BIG_ENDIAN_
		Dbuffer      = *(unsigned int *) Dcodestrm;
#else
		Dbuffer      = _bswap(*(unsigned int *) Dcodestrm);
#endif
		Dbits_to_go += 32;
		val         |= Dbuffer>>Dbits_to_go;
		Dcodestrm   += 4;
	}
	else
	{
		val          = Dbuffer>>Dbits_to_go;
	}
	return val&masks[numbits];
}
#else // _LEFT_JUSTIFY_
static inline unsigned int __fastcall H264GetBits PARGS1(int numbits)
{
	unsigned int val;

	val = Dbuffer>>(32-numbits);
	Dbuffer<<=numbits;
	Dbits_to_go -= numbits;
	if(Dbits_to_go<0)
	{
#ifdef _BIG_ENDIAN_
		Dbuffer      = *(unsigned int *) Dcodestrm;
#else
		Dbuffer      = _bswap(*(unsigned int *) Dcodestrm);
		//Dbuffer = Dcodestrm[0]<<24 | Dcodestrm[1]<<16 | Dcodestrm[2]<<8 | Dcodestrm[3];
#endif
		val         |= Dbuffer>>(32+Dbits_to_go);
		Dbuffer    <<= (-Dbits_to_go);
		Dcodestrm   += 4;
		Dbits_to_go += 32;
	}
	return val;
}
#endif // LEFT or RIGHT_JUSTIFY

/*!
************************************************************************
* \brief
*  Reads bits from the DecodingEnvironment buffer without advancing bit pointer
*
* \param numbits
*    number of bits to read
* \param img
*    image structure to access pointer to decoding engine state
*
************************************************************************
*/
#ifdef _RIGHT_JUSTIFY_
static inline unsigned int __fastcall H264ShowBits PARGS1(int numbits)
{
	unsigned int val;
	int lbits_to_go;

	lbits_to_go = Dbits_to_go - numbits;
	if(lbits_to_go<0)
	{
		unsigned int l_buffer;
		val      = Dbuffer<<(-lbits_to_go);
#ifdef _BIG_ENDIAN_
		l_buffer = *(unsigned int *) Dcodestrm;
#else
		l_buffer = _bswap(*(unsigned int *) Dcodestrm);
#endif
		val     |= l_buffer>>(32+lbits_to_go);
	}
	else
	{
		val      = Dbuffer>>lbits_to_go;
	}
	return val&masks[numbits];
}
#else // _LEFT_JUSTIFY_
static inline unsigned int __fastcall H264ShowBits PARGS1(int numbits)
{
	unsigned int val;
	int l_bits_to_go;

	val          = Dbuffer>>(32-numbits);
	l_bits_to_go = Dbits_to_go - numbits;
	if(l_bits_to_go<0)
	{
		unsigned int l_buffer;
#ifdef _BIG_ENDIAN_
		l_buffer = *(unsigned int *) Dcodestrm;
#else
		l_buffer = _bswap(*(unsigned int *) Dcodestrm);
#endif
		val     |= l_buffer>>(32+l_bits_to_go);
	}
	return val;
}
#endif // LEFT or RIGHT_JUSTIFY

/*!
************************************************************************
* \brief
*  Advances VLC decoding engine's bit pointer
*
* \param numbits
*    number of bits to read
* \param img
*    image structure to access pointer to decoding engine state
*
************************************************************************
*/
#ifdef _RIGHT_JUSTIFY_
static inline void __fastcall H264AdvanceBits PARGS1(int numbits)
{
	Dbits_to_go -=  numbits;
	if(Dbits_to_go<0)
	{
#ifdef _BIG_ENDIAN_
		Dbuffer      = *(unsigned int *) Dcodestrm;
#else
		Dbuffer      = _bswap(*(unsigned int *) Dcodestrm);
#endif
		Dcodestrm   += 4;
		Dbits_to_go += 32;
	}
}
#else // _LEFT_JUSTIFY_
static inline void __fastcall H264AdvanceBits PARGS1(int numbits)
{
	Dbuffer    <<=numbits;
	Dbits_to_go -= numbits;
	if(Dbits_to_go<0)
	{
#ifdef _BIG_ENDIAN_
		Dbuffer      = *(unsigned int *) Dcodestrm;
#else
		Dbuffer      = _bswap(*(unsigned int *) Dcodestrm);
#endif
		Dbuffer    <<= (-Dbits_to_go);
		Dcodestrm   += 4;
		Dbits_to_go += 32;
	}
}
#endif // LEFT or RIGHT_JUSTIFY

extern "C" const unsigned char __declspec(align(32)) times[512] GCC_ALIGN(32);

static inline int __fastcall find_leading_1 PARGS0()
{
#if 1
	int len, val;

	for(len = 0; (val = H264ShowBits ARGS1(8))==0; len += 8)
	{
		H264AdvanceBits ARGS1(8);
	}
	val = times[val];
	H264AdvanceBits ARGS1(val);
	len += val-1;

	return len;
#else
	int len;

	for(len = 0; H264GetBits ARGS1(1)==0; len++)
	{
	}
	return len;
#endif
}
/*!
************************************************************************
* \brief
*  read one exp-golomb VLC symbol
*
* \param buffer
*    containing VLC-coded data bits
* \param totbitoffset
*    bit offset from start of partition
* \param  info 
*    returns the value of the symbol
* \param bytecount
*    buffer length
* \return
*    bits read
************************************************************************
*/
int GetVLCSymbol PARGS1(int *info)
{
	int len;

	len = find_leading_1 ARGS0();

	if ( len!=0 )
		*info = H264GetBits ARGS1(len);
	else
		*info = 0;

	return len;           // return absolute offset in bit from start of frame
}

int GetVLCSymbol_IntraMode PARGS0()
{
	int ret_val;
	int ctr_bit;      // control bit for current bit posision

	ctr_bit = H264GetBits ARGS1(1);   // set up control bit

	//First bit
	if (ctr_bit) //len=1
	{
		ret_val = -1;
	}
	else //len=4
	{
		ret_val = H264GetBits ARGS1(3);
	}
	return ret_val;
}

/*!
************************************************************************
* \brief
*    read next UVLC codeword from UVLC-partition and
*    map it to the corresponding syntax element
************************************************************************
*/
int readSyntaxElement_VLC_ue PARGS0()
{
	int len, val;

	len =  GetVLCSymbol ARGS1(&val);

	return linfo_ue(len, val);
}

/*!
************************************************************************
* \brief
*    read next UVLC codeword from UVLC-partition and
*    map it to the corresponding syntax element
************************************************************************
*/
int readSyntaxElement_VLC_se PARGS0()
{
	int len, val;
	len =  GetVLCSymbol ARGS1(&val);
	return linfo_se(len, val);
}

/*!
************************************************************************
* \brief
*    read next UVLC codeword from UVLC-partition and
*    map it to the corresponding syntax element
************************************************************************
*/
int readSyntaxElement_VLC_cbp_intra PARGS0()
{
	int len, val;
	len = GetVLCSymbol ARGS1(&val);

	return linfo_cbp_intra ARGS2(len, val);
}

/*!
************************************************************************
* \brief
*    read next UVLC codeword from UVLC-partition and
*    map it to the corresponding syntax element
************************************************************************
*/
int readSyntaxElement_VLC_cbp_inter PARGS0()
{
	int len, val;
	len = GetVLCSymbol ARGS1(&val);

	return linfo_cbp_inter ARGS2(len, val);
}

/*!
************************************************************************
* \brief
*    read MVD x and y from UVLC-partition 
************************************************************************
*/
int readMVD_uvlc PARGS2(int list, MotionVector *mvd)
{
	//read x
	mvd->x = readSyntaxElement_VLC_se ARGS0();

	//read y
	mvd->y = readSyntaxElement_VLC_se ARGS0();

	return 1;
}

/*
************************************************************************
* \brief
*    read next UVLC codeword for MB type
*    
************************************************************************ 
*/
int read_raw_mb_uvlc PARGS0()
{
	return readSyntaxElement_VLC_ue ARGS0();
}

/*
************************************************************************
* \brief
*    read next UVLC codeword for DQuant type
*    
************************************************************************ 
*/
int read_raw_dquant_uvlc PARGS0()
{
	return readSyntaxElement_VLC_se ARGS0();
}

/*
************************************************************************
* \brief
*    read next UVLC codeword for CBP
*    
************************************************************************ 
*/
int read_raw_cbp_uvlc PARGS0()
{
	int ret_val;
	if (IMGPAR is_intra_block)
		ret_val = readSyntaxElement_VLC_cbp_intra ARGS0();
	else
		ret_val = readSyntaxElement_VLC_cbp_inter ARGS0();

	return ret_val;
}

/*!
************************************************************************
* \brief
*    read next VLC codeword for 4x4 Intra Prediction Mode and
*    map it to the corresponding Intra Prediction Direction
************************************************************************
*/
int readSyntaxElement_Intra4x4PredictionMode PARGS0()
{
	return GetVLCSymbol_IntraMode ARGS0();
}

/*!
************************************************************************
* \brief
*    test if bit buffer contains only stop bit
*
* \param buffer
*    buffer containing VLC-coded data bits
* \param totbitoffset
*    bit offset from start of partition
* \param bytecount
*    buffer length
* \return
*    true if more bits available
************************************************************************
*/
int more_rbsp_data PARGS0()
{
	int bytecount = Dstrmlength;
	long byteoffset;      // byte from start of buffer
	int bitoffset;      // bit from start of byte
	int ctr_bit;      // control bit for current bit posision

	byteoffset= (Dcodestrm - Dbasestrm) - ((Dbits_to_go+7)>>3);
	bitoffset= (8-Dbits_to_go)&7;

	//assert (byteoffset<bytecount);
	if (byteoffset >= bytecount) {
		return FALSE;	//Need improve
	}

	// there is more until we're in the last byte
	if (byteoffset<(bytecount-1)) 
		return TRUE;

	// read one bit
	ctr_bit = (H264ShowBits ARGS1(8)>>bitoffset)<<bitoffset;
	if(ctr_bit == 0x80)
		return FALSE;
	else
		return TRUE;
}


/*!
************************************************************************
* \brief
*    Check if there are symbols for the next MB
************************************************************************
*/
int uvlc_startcode_follows PARGS1(int dummy)
{
	//KS: new function test for End of Buffer
	return !(more_rbsp_data ARGS0());
}

/*!
************************************************************************
* \brief
*    read FLC codeword from UVLC-partition 
************************************************************************
*/
int readSyntaxElement_FLC PARGS1(int nbits)
{
	return H264GetBits ARGS1(nbits);
}

/*!
************************************************************************
* \brief
*    peek FLC codeword from UVLC-partition 
************************************************************************
*/
int peekSyntaxElement_FLC PARGS1(int nbits)
{
	return H264ShowBits ARGS1(nbits);
}

/************************************************************************
* \brief
*    get transform size flag from UVLC 
************************************************************************
*/

int read_transform_size_uvlc PARGS0()
{
	return(readSyntaxElement_FLC ARGS1(1));
}


//This is the node of Huffman tree vith 16-branches (4-bits) tree
typedef union
{
	struct {
		byte bits_num; //0 - we need to read more; > 0 - the leaf. It is actual number of bits
		byte value;    //depending on bits_num: Value for the leaf or index of the next 16 nodes
		//in the tree for branching
	};
	short comb;
} huf_decod_node_t;

/*!
************************************************************************
* \brief
*    read NumCoeff/TrailingOnes codeword from UVLC-partition 
************************************************************************
*/

int readSyntaxElement_NumCoeffTrailingOnes PARGS1(int vlcnum)
{
#if 0
	//VSoft
	static const huf_decod_node_t huf_tab0[] =
	{
		{0,  1}, {0, 14}, {3, 66}, {3, 66}, {2, 33}, {2, 33}, {2, 33}, {2, 33},
		{1,  0}, {1,  0}, {1,  0}, {1,  0}, {1,  0}, {1,  0}, {1,  0}, {1,  0},
		{0,  2}, {0, 11}, {0, 12}, {0, 13}, {4,102}, {4, 68}, {4, 35}, {4,  2},
		{3,101}, {3,101}, {3, 67}, {3, 67}, {2,100}, {2,100}, {2,100}, {2,100},
		{0,  3}, {0,  4}, {0,  5}, {0,  6}, {0,  7}, {0,  8}, {0,  9}, {0, 10},
		{3,105}, {3,105}, {3, 71}, {3, 71}, {3, 38}, {3, 38}, {3,  5}, {3,  5},
		{4,255}, {4,255}, {3, 45}, {3, 45}, {4, 16}, {4, 80}, {4, 48}, {4, 15},
		{4,112}, {4, 79}, {4, 47}, {4, 14}, {4,111}, {4, 78}, {4, 46}, {4, 13},
		{3,110}, {3,110}, {3, 77}, {3, 77}, {3, 44}, {3, 44}, {3, 12}, {3, 12},
		{3,109}, {3,109}, {3, 76}, {3, 76}, {3, 43}, {3, 43}, {3, 11}, {3, 11},
		{2,108}, {2,108}, {2,108}, {2,108}, {2, 75}, {2, 75}, {2, 75}, {2, 75},
		{2, 42}, {2, 42}, {2, 42}, {2, 42}, {2, 10}, {2, 10}, {2, 10}, {2, 10},
		{2,107}, {2,107}, {2,107}, {2,107}, {2, 74}, {2, 74}, {2, 74}, {2, 74},
		{2, 41}, {2, 41}, {2, 41}, {2, 41}, {2,  9}, {2,  9}, {2,  9}, {2,  9},
		{1,  8}, {1,  8}, {1,  8}, {1,  8}, {1,  8}, {1,  8}, {1,  8}, {1,  8},
		{1, 73}, {1, 73}, {1, 73}, {1, 73}, {1, 73}, {1, 73}, {1, 73}, {1, 73},
		{1, 40}, {1, 40}, {1, 40}, {1, 40}, {1, 40}, {1, 40}, {1, 40}, {1, 40},
		{1,  7}, {1,  7}, {1,  7}, {1,  7}, {1,  7}, {1,  7}, {1,  7}, {1,  7},
		{1,106}, {1,106}, {1,106}, {1,106}, {1,106}, {1,106}, {1,106}, {1,106},
		{1, 72}, {1, 72}, {1, 72}, {1, 72}, {1, 72}, {1, 72}, {1, 72}, {1, 72},
		{1, 39}, {1, 39}, {1, 39}, {1, 39}, {1, 39}, {1, 39}, {1, 39}, {1, 39},
		{1,  6}, {1,  6}, {1,  6}, {1,  6}, {1,  6}, {1,  6}, {1,  6}, {1,  6},
		{2,104}, {2,104}, {2,104}, {2,104}, {2, 70}, {2, 70}, {2, 70}, {2, 70},
		{2, 37}, {2, 37}, {2, 37}, {2, 37}, {2,  4}, {2,  4}, {2,  4}, {2,  4},
		{1,103}, {1,103}, {1,103}, {1,103}, {1,103}, {1,103}, {1,103}, {1,103},
		{1, 69}, {1, 69}, {1, 69}, {1, 69}, {1, 69}, {1, 69}, {1, 69}, {1, 69},
		{1, 36}, {1, 36}, {1, 36}, {1, 36}, {1, 36}, {1, 36}, {1, 36}, {1, 36},
		{1,  3}, {1,  3}, {1,  3}, {1,  3}, {1,  3}, {1,  3}, {1,  3}, {1,  3},
		{2, 34}, {2, 34}, {2, 34}, {2, 34}, {2,  1}, {2,  1}, {2,  1}, {2,  1},
		{1, 99}, {1, 99}, {1, 99}, {1, 99}, {1, 99}, {1, 99}, {1, 99}, {1, 99}
	};
	static const huf_decod_node_t huf_tab1[] =
	{
		{0,  1}, {0, 14}, {0, 15}, {0, 16}, {4,100}, {4, 99}, {3, 66}, {3, 66},
		{2, 33}, {2, 33}, {2, 33}, {2, 33}, {2,  0}, {2,  0}, {2,  0}, {2,  0},
		{0,  2}, {0, 11}, {0, 12}, {0, 13}, {4,  5}, {4, 70}, {4, 38}, {4,  4},
		{3,104}, {3,104}, {3, 69}, {3, 69}, {3, 37}, {3, 37}, {3,  3}, {3,  3},
		{0,  3}, {0,  4}, {0,  5}, {0,  6}, {0,  7}, {0,  8}, {0,  9}, {0, 10},
		{4, 11}, {4, 75}, {4, 43}, {4, 10}, {4,108}, {4, 74}, {4, 42}, {4,  9},
		{4,255}, {4,255}, {4,255}, {4,255}, {4,255}, {4,255}, {4,255}, {4,255},
		{1,111}, {1,111}, {1,111}, {1,111}, {1,111}, {1,111}, {1,111}, {1,111},
		{2,112}, {2,112}, {2,112}, {2,112}, {2, 80}, {2, 80}, {2, 80}, {2, 80},
		{2, 48}, {2, 48}, {2, 48}, {2, 48}, {2, 16}, {2, 16}, {2, 16}, {2, 16},
		{2, 47}, {2, 47}, {2, 47}, {2, 47}, {2, 15}, {2, 15}, {2, 15}, {2, 15},
		{2, 79}, {2, 79}, {2, 79}, {2, 79}, {2, 46}, {2, 46}, {2, 46}, {2, 46},
		{1, 78}, {1, 78}, {1, 78}, {1, 78}, {1, 78}, {1, 78}, {1, 78}, {1, 78},
		{1, 14}, {1, 14}, {1, 14}, {1, 14}, {1, 14}, {1, 14}, {1, 14}, {1, 14},
		{1,110}, {1,110}, {1,110}, {1,110}, {1,110}, {1,110}, {1,110}, {1,110},
		{1, 77}, {1, 77}, {1, 77}, {1, 77}, {1, 77}, {1, 77}, {1, 77}, {1, 77},
		{1, 45}, {1, 45}, {1, 45}, {1, 45}, {1, 45}, {1, 45}, {1, 45}, {1, 45},
		{1, 13}, {1, 13}, {1, 13}, {1, 13}, {1, 13}, {1, 13}, {1, 13}, {1, 13},
		{1,109}, {1,109}, {1,109}, {1,109}, {1,109}, {1,109}, {1,109}, {1,109},
		{1, 76}, {1, 76}, {1, 76}, {1, 76}, {1, 76}, {1, 76}, {1, 76}, {1, 76},
		{1, 44}, {1, 44}, {1, 44}, {1, 44}, {1, 44}, {1, 44}, {1, 44}, {1, 44},
		{1, 12}, {1, 12}, {1, 12}, {1, 12}, {1, 12}, {1, 12}, {1, 12}, {1, 12},
		{3,107}, {3,107}, {3, 73}, {3, 73}, {3, 41}, {3, 41}, {3,  8}, {3,  8},
		{3,106}, {3,106}, {3, 72}, {3, 72}, {3, 40}, {3, 40}, {3,  7}, {3,  7},
		{1,105}, {1,105}, {1,105}, {1,105}, {1,105}, {1,105}, {1,105}, {1,105},
		{1, 71}, {1, 71}, {1, 71}, {1, 71}, {1, 71}, {1, 71}, {1, 71}, {1, 71},
		{1, 39}, {1, 39}, {1, 39}, {1, 39}, {1, 39}, {1, 39}, {1, 39}, {1, 39},
		{1,  6}, {1,  6}, {1,  6}, {1,  6}, {1,  6}, {1,  6}, {1,  6}, {1,  6},
		{2,103}, {2,103}, {2,103}, {2,103}, {2, 68}, {2, 68}, {2, 68}, {2, 68},
		{2, 36}, {2, 36}, {2, 36}, {2, 36}, {2,  2}, {2,  2}, {2,  2}, {2,  2},
		{2,102}, {2,102}, {2,102}, {2,102}, {2, 67}, {2, 67}, {2, 67}, {2, 67},
		{2, 35}, {2, 35}, {2, 35}, {2, 35}, {2,  1}, {2,  1}, {2,  1}, {2,  1},
		{1,101}, {1,101}, {1,101}, {1,101}, {1,101}, {1,101}, {1,101}, {1,101},
		{1, 34}, {1, 34}, {1, 34}, {1, 34}, {1, 34}, {1, 34}, {1, 34}, {1, 34}
	};
	static const huf_decod_node_t huf_tab2[] =
	{
		{0,  1}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15}, {0, 16},
		{4,103}, {4,102}, {4,101}, {4,100}, {4, 99}, {4, 66}, {4, 33}, {4,  0},
		{0,  2}, {0,  3}, {0,  4}, {0,  5}, {0,  6}, {0,  7}, {0,  8}, {0,  9},
		{4,108}, {4, 75}, {4, 42}, {4,  9}, {4,107}, {4, 74}, {4, 41}, {4,  8},
		{4,255}, {4,255}, {4,255}, {4,255}, {2, 16}, {2, 16}, {2, 16}, {2, 16},
		{2,112}, {2,112}, {2,112}, {2,112}, {2, 80}, {2, 80}, {2, 80}, {2, 80},
		{2, 48}, {2, 48}, {2, 48}, {2, 48}, {2, 15}, {2, 15}, {2, 15}, {2, 15},
		{2,111}, {2,111}, {2,111}, {2,111}, {2, 79}, {2, 79}, {2, 79}, {2, 79},
		{2, 47}, {2, 47}, {2, 47}, {2, 47}, {2, 14}, {2, 14}, {2, 14}, {2, 14},
		{2,110}, {2,110}, {2,110}, {2,110}, {2, 78}, {2, 78}, {2, 78}, {2, 78},
		{2, 46}, {2, 46}, {2, 46}, {2, 46}, {2, 13}, {2, 13}, {2, 13}, {2, 13},
		{1, 45}, {1, 45}, {1, 45}, {1, 45}, {1, 45}, {1, 45}, {1, 45}, {1, 45},
		{1, 12}, {1, 12}, {1, 12}, {1, 12}, {1, 12}, {1, 12}, {1, 12}, {1, 12},
		{1, 77}, {1, 77}, {1, 77}, {1, 77}, {1, 77}, {1, 77}, {1, 77}, {1, 77},
		{1, 44}, {1, 44}, {1, 44}, {1, 44}, {1, 44}, {1, 44}, {1, 44}, {1, 44},
		{1, 11}, {1, 11}, {1, 11}, {1, 11}, {1, 11}, {1, 11}, {1, 11}, {1, 11},
		{1,109}, {1,109}, {1,109}, {1,109}, {1,109}, {1,109}, {1,109}, {1,109},
		{1, 76}, {1, 76}, {1, 76}, {1, 76}, {1, 76}, {1, 76}, {1, 76}, {1, 76},
		{1, 43}, {1, 43}, {1, 43}, {1, 43}, {1, 43}, {1, 43}, {1, 43}, {1, 43},
		{1, 10}, {1, 10}, {1, 10}, {1, 10}, {1, 10}, {1, 10}, {1, 10}, {1, 10},
		{3,  7}, {3,  7}, {3,  6}, {3,  6}, {3, 73}, {3, 73}, {3,  5}, {3,  5},
		{3,106}, {3,106}, {3, 72}, {3, 72}, {3, 40}, {3, 40}, {3,  4}, {3,  4},
		{2,  3}, {2,  3}, {2,  3}, {2,  3}, {2, 71}, {2, 71}, {2, 71}, {2, 71},
		{2, 39}, {2, 39}, {2, 39}, {2, 39}, {2,  2}, {2,  2}, {2,  2}, {2,  2},
		{2,105}, {2,105}, {2,105}, {2,105}, {2, 70}, {2, 70}, {2, 70}, {2, 70},
		{2, 38}, {2, 38}, {2, 38}, {2, 38}, {2,  1}, {2,  1}, {2,  1}, {2,  1},
		{1, 37}, {1, 37}, {1, 37}, {1, 37}, {1, 37}, {1, 37}, {1, 37}, {1, 37},
		{1, 69}, {1, 69}, {1, 69}, {1, 69}, {1, 69}, {1, 69}, {1, 69}, {1, 69},
		{1, 36}, {1, 36}, {1, 36}, {1, 36}, {1, 36}, {1, 36}, {1, 36}, {1, 36},
		{1, 68}, {1, 68}, {1, 68}, {1, 68}, {1, 68}, {1, 68}, {1, 68}, {1, 68},
		{1, 35}, {1, 35}, {1, 35}, {1, 35}, {1, 35}, {1, 35}, {1, 35}, {1, 35},
		{1,104}, {1,104}, {1,104}, {1,104}, {1,104}, {1,104}, {1,104}, {1,104},
		{1, 67}, {1, 67}, {1, 67}, {1, 67}, {1, 67}, {1, 67}, {1, 67}, {1, 67},
		{1, 34}, {1, 34}, {1, 34}, {1, 34}, {1, 34}, {1, 34}, {1, 34}, {1, 34}
	};

	static const byte flc_tab3[64] = {
		(0<<5)|( 1),(1<<5)|( 1),(2<<5)|( 1),(0<<5)|( 0),
		(0<<5)|( 2),(1<<5)|( 2),(2<<5)|( 2),(3<<5)|( 2),
		(0<<5)|( 3),(1<<5)|( 3),(2<<5)|( 3),(3<<5)|( 3),
		(0<<5)|( 4),(1<<5)|( 4),(2<<5)|( 4),(3<<5)|( 4),
		(0<<5)|( 5),(1<<5)|( 5),(2<<5)|( 5),(3<<5)|( 5),
		(0<<5)|( 6),(1<<5)|( 6),(2<<5)|( 6),(3<<5)|( 6),
		(0<<5)|( 7),(1<<5)|( 7),(2<<5)|( 7),(3<<5)|( 7),
		(0<<5)|( 8),(1<<5)|( 8),(2<<5)|( 8),(3<<5)|( 8),
		(0<<5)|( 9),(1<<5)|( 9),(2<<5)|( 9),(3<<5)|( 9),
		(0<<5)|(10),(1<<5)|(10),(2<<5)|(10),(3<<5)|(10),
		(0<<5)|(11),(1<<5)|(11),(2<<5)|(11),(3<<5)|(11),
		(0<<5)|(12),(1<<5)|(12),(2<<5)|(12),(3<<5)|(12),
		(0<<5)|(13),(1<<5)|(13),(2<<5)|(13),(3<<5)|(13),
		(0<<5)|(14),(1<<5)|(14),(2<<5)|(14),(3<<5)|(14),
		(0<<5)|(15),(1<<5)|(15),(2<<5)|(15),(3<<5)|(15),
		(0<<5)|(16),(1<<5)|(16),(2<<5)|(16),(3<<5)|(16) };

		static const huf_decod_node_t *huf_tab[] = { huf_tab0, huf_tab1, huf_tab2 };

		huf_decod_node_t *table;

		// vlcnum is the index of Table used to code coeff_token
		// vlcnum==3 means (8<=nC) which uses 6bit FLC

		if (vlcnum == 3)
		{
			// read 6 bit FLC
			return flc_tab3[H264GetBits ARGS1(6)];
		}
		else
		{
#ifdef _ARM_FIX_
			unsigned char (*table)[2] = reinterpret_cast<unsigned char (*)[2]>(huf_tab[vlcnum]);

			register int pos = H264ShowBits ARGS1(4);

			int bits_num = table[pos][0];

			while (bits_num == 0 )
			{
				H264AdvanceBits ARGS1(4);
				pos = (table[pos][1]<<4) + H264ShowBits ARGS1(4);
				bits_num = table[pos][0];
				if (bits_num > 0 && table[pos][1]==255) 
				{ //error
					return 0;
				}
			}
			H264AdvanceBits ARGS1(bits_num);
			return table[pos][1];
#else
			table = (huf_decod_node_t *) huf_tab[vlcnum];

			register int pos = H264ShowBits ARGS1(4);

			int bits_num = table[pos].bits_num;
			while (bits_num == 0 )
			{
				H264AdvanceBits ARGS1(4);
				pos = (table[pos].value<<4) + H264ShowBits ARGS1(4);
				bits_num = table[pos].bits_num;
				if (bits_num > 0 && table[pos].value==255) 
				{ //error
					return 0;
				}
			}
			H264AdvanceBits ARGS1(bits_num);
			return table[pos].value;
#endif //_ARM_FIX_
		}

#else

	// New table - 6+5+5
	static const unsigned short huf_tab0[64+4*32+4*32] =
	{
		// First 64 entries, first 6 bits
		_C2L(0, 0,0), _C2L(1, 0,0), _C2L(2, 0,0), _C2L(3, 4,6), _C2L(1, 2,6), _C2L(0, 1,6), _C2L(3, 3,5), _C2L(3, 3,5),
		_C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3),
		_C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2),
		_C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2),
		_C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1),
		_C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1),
		_C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1),
		_C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1), _C2L(0, 0,1),

		// Next 32 entries, second 5 bits, (0)
		_C2L(0, 0,0), _C2L(1, 0,0), _C2L(2, 0,0), _C2L(3, 0,0), _C2L(3, 9,5), _C2L(2, 7,5), _C2L(1, 6,5), _C2L(0, 5,5),
		_C2L(3, 8,4), _C2L(3, 8,4), _C2L(2, 6,4), _C2L(2, 6,4), _C2L(1, 5,4), _C2L(1, 5,4), _C2L(0, 4,4), _C2L(0, 4,4),
		_C2L(3, 7,3), _C2L(3, 7,3), _C2L(3, 7,3), _C2L(3, 7,3), _C2L(2, 5,3), _C2L(2, 5,3), _C2L(2, 5,3), _C2L(2, 5,3),
		_C2L(1, 4,3), _C2L(1, 4,3), _C2L(1, 4,3), _C2L(1, 4,3), _C2L(0, 3,3), _C2L(0, 3,3), _C2L(0, 3,3), _C2L(0, 3,3),
		// Next 32 entries, second 5 bits, (1)
		_C2L(3, 6,2), _C2L(3, 6,2), _C2L(3, 6,2), _C2L(3, 6,2), _C2L(3, 6,2), _C2L(3, 6,2), _C2L(3, 6,2), _C2L(3, 6,2),
		_C2L(2, 4,2), _C2L(2, 4,2), _C2L(2, 4,2), _C2L(2, 4,2), _C2L(2, 4,2), _C2L(2, 4,2), _C2L(2, 4,2), _C2L(2, 4,2),
		_C2L(1, 3,2), _C2L(1, 3,2), _C2L(1, 3,2), _C2L(1, 3,2), _C2L(1, 3,2), _C2L(1, 3,2), _C2L(1, 3,2), _C2L(1, 3,2),
		_C2L(0, 2,2), _C2L(0, 2,2), _C2L(0, 2,2), _C2L(0, 2,2), _C2L(0, 2,2), _C2L(0, 2,2), _C2L(0, 2,2), _C2L(0, 2,2),
		// Next 32 entries, second 5 bits, (2)
		_C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1),
		_C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1), _C2L(3, 5,1),
		_C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1),
		_C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1), _C2L(2, 3,1),
		// Next 32 entries, second 5 bits, (3) - will never happen
		_C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0),
		_C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0),
		_C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0),
		_C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0), _C2L(0, 0,0),

		// Next 32 entries, third 5 bits, (0, 0)
		_C2L(7,63,5), _C2L(7,63,5), _C2L(1,13,4), _C2L(1,13,4), _C2L(0,16,5), _C2L(2,16,5), _C2L(1,16,5), _C2L(0,15,5),
		_C2L(3,16,5), _C2L(2,15,5), _C2L(1,15,5), _C2L(0,14,5), _C2L(3,15,5), _C2L(2,14,5), _C2L(1,14,5), _C2L(0,13,5),
		_C2L(3,14,4), _C2L(3,14,4), _C2L(2,13,4), _C2L(2,13,4), _C2L(1,12,4), _C2L(1,12,4), _C2L(0,12,4), _C2L(0,12,4),
		_C2L(3,13,4), _C2L(3,13,4), _C2L(2,12,4), _C2L(2,12,4), _C2L(1,11,4), _C2L(1,11,4), _C2L(0,11,4), _C2L(0,11,4),
		// Next 32 entries, third 5 bits, (0, 1)
		_C2L(3,12,3), _C2L(3,12,3), _C2L(3,12,3), _C2L(3,12,3), _C2L(2,11,3), _C2L(2,11,3), _C2L(2,11,3), _C2L(2,11,3),
		_C2L(1,10,3), _C2L(1,10,3), _C2L(1,10,3), _C2L(1,10,3), _C2L(0,10,3), _C2L(0,10,3), _C2L(0,10,3), _C2L(0,10,3),
		_C2L(3,11,3), _C2L(3,11,3), _C2L(3,11,3), _C2L(3,11,3), _C2L(2,10,3), _C2L(2,10,3), _C2L(2,10,3), _C2L(2,10,3),
		_C2L(1, 9,3), _C2L(1, 9,3), _C2L(1, 9,3), _C2L(1, 9,3), _C2L(0, 9,3), _C2L(0, 9,3), _C2L(0, 9,3), _C2L(0, 9,3),
		// Next 32 entries, third 5 bits, (0, 2)
		_C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2),
		_C2L(2, 9,2), _C2L(2, 9,2), _C2L(2, 9,2), _C2L(2, 9,2), _C2L(2, 9,2), _C2L(2, 9,2), _C2L(2, 9,2), _C2L(2, 9,2),
		_C2L(1, 8,2), _C2L(1, 8,2), _C2L(1, 8,2), _C2L(1, 8,2), _C2L(1, 8,2), _C2L(1, 8,2), _C2L(1, 8,2), _C2L(1, 8,2),
		_C2L(0, 7,2), _C2L(0, 7,2), _C2L(0, 7,2), _C2L(0, 7,2), _C2L(0, 7,2), _C2L(0, 7,2), _C2L(0, 7,2), _C2L(0, 7,2),
		// Next 32 entries, third 5 bits, (0, 3)
		_C2L(3,10,2), _C2L(3,10,2), _C2L(3,10,2), _C2L(3,10,2), _C2L(3,10,2), _C2L(3,10,2), _C2L(3,10,2), _C2L(3,10,2),
		_C2L(2, 8,2), _C2L(2, 8,2), _C2L(2, 8,2), _C2L(2, 8,2), _C2L(2, 8,2), _C2L(2, 8,2), _C2L(2, 8,2), _C2L(2, 8,2),
		_C2L(1, 7,2), _C2L(1, 7,2), _C2L(1, 7,2), _C2L(1, 7,2), _C2L(1, 7,2), _C2L(1, 7,2), _C2L(1, 7,2), _C2L(1, 7,2),
		_C2L(0, 6,2), _C2L(0, 6,2), _C2L(0, 6,2), _C2L(0, 6,2), _C2L(0, 6,2), _C2L(0, 6,2), _C2L(0, 6,2), _C2L(0, 6,2)
	};

	// New table - 6+5+5
	static const unsigned short huf_tab1[64+4*32+8*32] =
	{
		// First 64 entries, first 6 bits
		_C2L(0, 0,0), _C2L(1, 0,0), _C2L(2, 0,0), _C2L(3, 0,0), _C2L(3, 7,6), _C2L(2, 4,6), _C2L(1, 4,6), _C2L(0, 2,6),
		_C2L(3, 6,6), _C2L(2, 3,6), _C2L(1, 3,6), _C2L(0, 1,6), _C2L(3, 5,5), _C2L(3, 5,5), _C2L(1, 2,5), _C2L(1, 2,5),
		_C2L(3, 4,4), _C2L(3, 4,4), _C2L(3, 4,4), _C2L(3, 4,4), _C2L(3, 3,4), _C2L(3, 3,4), _C2L(3, 3,4), _C2L(3, 3,4),
		_C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3), _C2L(2, 2,3),
		_C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2),
		_C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2), _C2L(1, 1,2),
		_C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2),
		_C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2), _C2L(0, 0,2),

		// Next 32 entries, second 5 bits, (0)
		_C2L(0, 0,0), _C2L(1, 0,0), _C2L(2, 0,0), _C2L(3, 0,0), _C2L(4, 0,0), _C2L(5, 0,0), _C2L(6, 0,0), _C2L(7, 0,0),
		_C2L(3,11,5), _C2L(2, 9,5), _C2L(1, 9,5), _C2L(0, 8,5), _C2L(3,10,5), _C2L(2, 8,5), _C2L(1, 8,5), _C2L(0, 7,5),
		_C2L(3, 9,3), _C2L(3, 9,3), _C2L(3, 9,3), _C2L(3, 9,3), _C2L(2, 7,3), _C2L(2, 7,3), _C2L(2, 7,3), _C2L(2, 7,3),
		_C2L(1, 7,3), _C2L(1, 7,3), _C2L(1, 7,3), _C2L(1, 7,3), _C2L(0, 6,3), _C2L(0, 6,3), _C2L(0, 6,3), _C2L(0, 6,3),
		// Next 32 entries, second 5 bits, (1)
		_C2L(0, 5,2), _C2L(0, 5,2), _C2L(0, 5,2), _C2L(0, 5,2), _C2L(0, 5,2), _C2L(0, 5,2), _C2L(0, 5,2), _C2L(0, 5,2),
		_C2L(2, 6,2), _C2L(2, 6,2), _C2L(2, 6,2), _C2L(2, 6,2), _C2L(2, 6,2), _C2L(2, 6,2), _C2L(2, 6,2), _C2L(2, 6,2),
		_C2L(1, 6,2), _C2L(1, 6,2), _C2L(1, 6,2), _C2L(1, 6,2), _C2L(1, 6,2), _C2L(1, 6,2), _C2L(1, 6,2), _C2L(1, 6,2),
		_C2L(0, 4,2), _C2L(0, 4,2), _C2L(0, 4,2), _C2L(0, 4,2), _C2L(0, 4,2), _C2L(0, 4,2), _C2L(0, 4,2), _C2L(0, 4,2),
		// Next 32 entries, second 5 bits, (2)
		_C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1),
		_C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1), _C2L(3, 8,1),
		_C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1),
		_C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1), _C2L(2, 5,1),
		// Next 32 entries, second 5 bits, (3)
		_C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1),
		_C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1), _C2L(1, 5,1),
		_C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1),
		_C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1), _C2L(0, 3,1),

		// Next 32 entries, third 5 bits, (0, 0)
		_C2L(7,63,5), _C2L(7,63,5), _C2L(7,63,5), _C2L(7,63,5), _C2L(7,63,5), _C2L(7,63,5), _C2L(7,63,5), _C2L(7,63,5),
		_C2L(3,15,2), _C2L(3,15,2), _C2L(3,15,2), _C2L(3,15,2), _C2L(3,15,2), _C2L(3,15,2), _C2L(3,15,2), _C2L(3,15,2),
		_C2L(3,16,3), _C2L(3,16,3), _C2L(3,16,3), _C2L(3,16,3), _C2L(2,16,3), _C2L(2,16,3), _C2L(2,16,3), _C2L(2,16,3),
		_C2L(1,16,3), _C2L(1,16,3), _C2L(1,16,3), _C2L(1,16,3), _C2L(0,16,3), _C2L(0,16,3), _C2L(0,16,3), _C2L(0,16,3),
		// Next 32 entries, third 5 bits, (0, 1)
		_C2L(1,15,3), _C2L(1,15,3), _C2L(1,15,3), _C2L(1,15,3), _C2L(0,15,3), _C2L(0,15,3), _C2L(0,15,3), _C2L(0,15,3),
		_C2L(2,15,3), _C2L(2,15,3), _C2L(2,15,3), _C2L(2,15,3), _C2L(1,14,3), _C2L(1,14,3), _C2L(1,14,3), _C2L(1,14,3),
		_C2L(2,14,2), _C2L(2,14,2), _C2L(2,14,2), _C2L(2,14,2), _C2L(2,14,2), _C2L(2,14,2), _C2L(2,14,2), _C2L(2,14,2),
		_C2L(0,14,2), _C2L(0,14,2), _C2L(0,14,2), _C2L(0,14,2), _C2L(0,14,2), _C2L(0,14,2), _C2L(0,14,2), _C2L(0,14,2),
		// Next 32 entries, third 5 bits, (0, 2)
		_C2L(3,14,2), _C2L(3,14,2), _C2L(3,14,2), _C2L(3,14,2), _C2L(3,14,2), _C2L(3,14,2), _C2L(3,14,2), _C2L(3,14,2),
		_C2L(2,13,2), _C2L(2,13,2), _C2L(2,13,2), _C2L(2,13,2), _C2L(2,13,2), _C2L(2,13,2), _C2L(2,13,2), _C2L(2,13,2),
		_C2L(1,13,2), _C2L(1,13,2), _C2L(1,13,2), _C2L(1,13,2), _C2L(1,13,2), _C2L(1,13,2), _C2L(1,13,2), _C2L(1,13,2),
		_C2L(0,13,2), _C2L(0,13,2), _C2L(0,13,2), _C2L(0,13,2), _C2L(0,13,2), _C2L(0,13,2), _C2L(0,13,2), _C2L(0,13,2),
		// Next 32 entries, third 5 bits, (0, 3)
		_C2L(3,13,2), _C2L(3,13,2), _C2L(3,13,2), _C2L(3,13,2), _C2L(3,13,2), _C2L(3,13,2), _C2L(3,13,2), _C2L(3,13,2),
		_C2L(2,12,2), _C2L(2,12,2), _C2L(2,12,2), _C2L(2,12,2), _C2L(2,12,2), _C2L(2,12,2), _C2L(2,12,2), _C2L(2,12,2),
		_C2L(1,12,2), _C2L(1,12,2), _C2L(1,12,2), _C2L(1,12,2), _C2L(1,12,2), _C2L(1,12,2), _C2L(1,12,2), _C2L(1,12,2),
		_C2L(0,12,2), _C2L(0,12,2), _C2L(0,12,2), _C2L(0,12,2), _C2L(0,12,2), _C2L(0,12,2), _C2L(0,12,2), _C2L(0,12,2),
		// Next 32 entries, third 5 bits, (0, 4)
		_C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1),
		_C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1), _C2L(0,11,1),
		_C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1),
		_C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1), _C2L(2,11,1),
		// Next 32 entries, third 5 bits, (0, 5)
		_C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1),
		_C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1), _C2L(1,11,1),
		_C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1),
		_C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1), _C2L(0,10,1),
		// Next 32 entries, third 5 bits, (0, 6)
		_C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1),
		_C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1), _C2L(3,12,1),
		_C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1),
		_C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1), _C2L(2,10,1),
		// Next 32 entries, third 5 bits, (0, 7)
		_C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1),
		_C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1), _C2L(1,10,1),
		_C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1),
		_C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1), _C2L(0, 9,1)
	};

	// New table - 6+5+5
	static const unsigned short huf_tab2[64+8*32] =
	{
		// First 64 entries, first 6 bits
		_C2L(0, 0,0), _C2L(1, 0,0), _C2L(2, 0,0), _C2L(3, 0,0), _C2L(4, 0,0), _C2L(5, 0,0), _C2L(6, 0,0), _C2L(7, 0,0),
		_C2L(0, 3,6), _C2L(2, 7,6), _C2L(1, 7,6), _C2L(0, 2,6), _C2L(3, 9,6), _C2L(2, 6,6), _C2L(1, 6,6), _C2L(0, 1,6),
		_C2L(1, 5,5), _C2L(1, 5,5), _C2L(2, 5,5), _C2L(2, 5,5), _C2L(1, 4,5), _C2L(1, 4,5), _C2L(2, 4,5), _C2L(2, 4,5),
		_C2L(1, 3,5), _C2L(1, 3,5), _C2L(3, 8,5), _C2L(3, 8,5), _C2L(2, 3,5), _C2L(2, 3,5), _C2L(1, 2,5), _C2L(1, 2,5),
		_C2L(3, 7,4), _C2L(3, 7,4), _C2L(3, 7,4), _C2L(3, 7,4), _C2L(3, 6,4), _C2L(3, 6,4), _C2L(3, 6,4), _C2L(3, 6,4),
		_C2L(3, 5,4), _C2L(3, 5,4), _C2L(3, 5,4), _C2L(3, 5,4), _C2L(3, 4,4), _C2L(3, 4,4), _C2L(3, 4,4), _C2L(3, 4,4),
		_C2L(3, 3,4), _C2L(3, 3,4), _C2L(3, 3,4), _C2L(3, 3,4), _C2L(2, 2,4), _C2L(2, 2,4), _C2L(2, 2,4), _C2L(2, 2,4),
		_C2L(1, 1,4), _C2L(1, 1,4), _C2L(1, 1,4), _C2L(1, 1,4), _C2L(0, 0,4), _C2L(0, 0,4), _C2L(0, 0,4), _C2L(0, 0,4),

		// Next 32 entries, second 5 bits, (0)
		_C2L(7,63,5), _C2L(7,63,5), _C2L(0,16,4), _C2L(0,16,4), _C2L(3,16,4), _C2L(3,16,4), _C2L(2,16,4), _C2L(2,16,4),
		_C2L(1,16,4), _C2L(1,16,4), _C2L(0,15,4), _C2L(0,15,4), _C2L(3,15,4), _C2L(3,15,4), _C2L(2,15,4), _C2L(2,15,4),
		_C2L(1,15,4), _C2L(1,15,4), _C2L(0,14,4), _C2L(0,14,4), _C2L(3,14,4), _C2L(3,14,4), _C2L(2,14,4), _C2L(2,14,4),
		_C2L(1,14,4), _C2L(1,14,4), _C2L(0,13,4), _C2L(0,13,4), _C2L(1,13,3), _C2L(1,13,3), _C2L(1,13,3), _C2L(1,13,3),
		// Next 32 entries, second 5 bits, (1)
		_C2L(0,12,3), _C2L(0,12,3), _C2L(0,12,3), _C2L(0,12,3), _C2L(2,13,3), _C2L(2,13,3), _C2L(2,13,3), _C2L(2,13,3),
		_C2L(1,12,3), _C2L(1,12,3), _C2L(1,12,3), _C2L(1,12,3), _C2L(0,11,3), _C2L(0,11,3), _C2L(0,11,3), _C2L(0,11,3),
		_C2L(3,13,3), _C2L(3,13,3), _C2L(3,13,3), _C2L(3,13,3), _C2L(2,12,3), _C2L(2,12,3), _C2L(2,12,3), _C2L(2,12,3),
		_C2L(1,11,3), _C2L(1,11,3), _C2L(1,11,3), _C2L(1,11,3), _C2L(0,10,3), _C2L(0,10,3), _C2L(0,10,3), _C2L(0,10,3),
		// Next 32 entries, second 5 bits, (2)
		_C2L(3,12,2), _C2L(3,12,2), _C2L(3,12,2), _C2L(3,12,2), _C2L(3,12,2), _C2L(3,12,2), _C2L(3,12,2), _C2L(3,12,2),
		_C2L(2,11,2), _C2L(2,11,2), _C2L(2,11,2), _C2L(2,11,2), _C2L(2,11,2), _C2L(2,11,2), _C2L(2,11,2), _C2L(2,11,2),
		_C2L(1,10,2), _C2L(1,10,2), _C2L(1,10,2), _C2L(1,10,2), _C2L(1,10,2), _C2L(1,10,2), _C2L(1,10,2), _C2L(1,10,2),
		_C2L(0, 9,2), _C2L(0, 9,2), _C2L(0, 9,2), _C2L(0, 9,2), _C2L(0, 9,2), _C2L(0, 9,2), _C2L(0, 9,2), _C2L(0, 9,2),
		// Next 32 entries, second 5 bits, (3)
		_C2L(3,11,2), _C2L(3,11,2), _C2L(3,11,2), _C2L(3,11,2), _C2L(3,11,2), _C2L(3,11,2), _C2L(3,11,2), _C2L(3,11,2),
		_C2L(2,10,2), _C2L(2,10,2), _C2L(2,10,2), _C2L(2,10,2), _C2L(2,10,2), _C2L(2,10,2), _C2L(2,10,2), _C2L(2,10,2),
		_C2L(1, 9,2), _C2L(1, 9,2), _C2L(1, 9,2), _C2L(1, 9,2), _C2L(1, 9,2), _C2L(1, 9,2), _C2L(1, 9,2), _C2L(1, 9,2),
		_C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2), _C2L(0, 8,2),
		// Next 32 entries, third 5 bits, (4)
		_C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1),
		_C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1), _C2L(0, 7,1),
		_C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1),
		_C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1), _C2L(0, 6,1),
		// Next 32 entries, third 5 bits, (5)
		_C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1),
		_C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1), _C2L(2, 9,1),
		_C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1),
		_C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1), _C2L(0, 5,1),
		// Next 32 entries, third 5 bits, (6)
		_C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1),
		_C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1), _C2L(3,10,1),
		_C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1),
		_C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1), _C2L(2, 8,1),
		// Next 32 entries, third 5 bits, (7)
		_C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1),
		_C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1), _C2L(1, 8,1),
		_C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1),
		_C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1), _C2L(0, 4,1)
	};

	static const unsigned short huf_tab3[64] =
	{
		_C2L(0, 1,6), _C2L(1, 1,6), _C2L(2, 1,6), _C2L(0, 0,6),
		_C2L(0, 2,6), _C2L(1, 2,6), _C2L(2, 2,6), _C2L(3, 2,6),
		_C2L(0, 3,6), _C2L(1, 3,6), _C2L(2, 3,6), _C2L(3, 3,6),
		_C2L(0, 4,6), _C2L(1, 4,6), _C2L(2, 4,6), _C2L(3, 4,6),
		_C2L(0, 5,6), _C2L(1, 5,6), _C2L(2, 5,6), _C2L(3, 5,6),
		_C2L(0, 6,6), _C2L(1, 6,6), _C2L(2, 6,6), _C2L(3, 6,6),
		_C2L(0, 7,6), _C2L(1, 7,6), _C2L(2, 7,6), _C2L(3, 7,6),
		_C2L(0, 8,6), _C2L(1, 8,6), _C2L(2, 8,6), _C2L(3, 8,6),
		_C2L(0, 9,6), _C2L(1, 9,6), _C2L(2, 9,6), _C2L(3, 9,6),
		_C2L(0,10,6), _C2L(1,10,6), _C2L(2,10,6), _C2L(3,10,6),
		_C2L(0,11,6), _C2L(1,11,6), _C2L(2,11,6), _C2L(3,11,6),
		_C2L(0,12,6), _C2L(1,12,6), _C2L(2,12,6), _C2L(3,12,6),
		_C2L(0,13,6), _C2L(1,13,6), _C2L(2,13,6), _C2L(3,13,6),
		_C2L(0,14,6), _C2L(1,14,6), _C2L(2,14,6), _C2L(3,14,6),
		_C2L(0,15,6), _C2L(1,15,6), _C2L(2,15,6), _C2L(3,15,6),
		_C2L(0,16,6), _C2L(1,16,6), _C2L(2,16,6), _C2L(3,16,6)
	};

	static const unsigned short *huf_tab[] = { huf_tab0, huf_tab1, huf_tab2, huf_tab3 };

#ifdef _ARM_FIX_
	const unsigned char (*table)[2] = reinterpret_cast<const unsigned char (*)[2]>(huf_tab[vlcnum]);

	register int pos = H264ShowBits ARGS1(6);

	int bits_num = table[pos][0];
	if (bits_num == 0)
	{
		H264AdvanceBits ARGS1(6);
		pos = 64 + table[pos][1] + H264ShowBits ARGS1(5);
		bits_num = table[pos][0];
		// We should check that table[pos][1] != 255 for error checking
		if (bits_num == 0)
		{
			H264AdvanceBits ARGS1(5);
			pos = 64 + 32*4 + table[pos][1] + H264ShowBits ARGS1(5);
			bits_num = table[pos][0];
			// We should check that table[pos][1] != 255 for error checking
		}
	}
	H264AdvanceBits ARGS1(bits_num);
	return table[pos][1];

#else
	huf_decod_node_t *table;

	table = (huf_decod_node_t *) huf_tab[vlcnum];

	register int pos = H264ShowBits ARGS1(6);

	int bits_num = table[pos].bits_num;
	if (bits_num == 0)
	{
		H264AdvanceBits ARGS1(6);
		pos = 64 + table[pos].value + H264ShowBits ARGS1(5);
		bits_num = table[pos].bits_num;
		// We should check that table[pos].value != 255 for error checking
		if (bits_num == 0)
		{
			H264AdvanceBits ARGS1(5);
			pos = 64 + 32*4 + table[pos].value + H264ShowBits ARGS1(5);
			bits_num = table[pos].bits_num;
			// We should check that table[pos].value != 255 for error checking
		}
	}
	H264AdvanceBits ARGS1(bits_num);
	return table[pos].value;

#endif //_ARM_FIX_
#endif

}

/*!
************************************************************************
* \brief
*    read NumCoeff/TrailingOnes codeword from UVLC-partition ChromaDC
************************************************************************
*/
int readSyntaxElement_NumCoeffTrailingOnesChromaDC PARGS0()
{
	int code;

	static const unsigned short vlctab[256] = 
	{	  
		_C2L(3,4,7),_C2L(3,4,7),_C2L(2,4,8),_C2L(1,4,8),	
		_C2L(2,3,7),_C2L(2,3,7),_C2L(1,3,7),_C2L(1,3,7),		
		_C2L(0,4,6),_C2L(0,4,6),_C2L(0,4,6),_C2L(0,4,6),
		_C2L(0,3,6),_C2L(0,3,6),_C2L(0,3,6),_C2L(0,3,6),
		_C2L(0,2,6),_C2L(0,2,6),_C2L(0,2,6),_C2L(0,2,6),
		_C2L(3,3,6),_C2L(3,3,6),_C2L(3,3,6),_C2L(3,3,6),
		_C2L(1,2,6),_C2L(1,2,6),_C2L(1,2,6),_C2L(1,2,6),
		_C2L(0,1,6),_C2L(0,1,6),_C2L(0,1,6),_C2L(0,1,6),

		_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),
		_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),
		_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),
		_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),
		_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),
		_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),
		_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),
		_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),_C2L(2,2,3),

		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),

		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),
		_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),_C2L(0,0,2),

		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),

		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),

		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),

		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),
		_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),_C2L(1,1,1),

	};

#if 0
#ifdef _ARM_FIX_
	const unsigned char (*table)[2] = reinterpret_cast<const unsigned char (*)[2]>(vlctab);

	code = H264ShowBits ARGS1(8);//PeekBits(8);
	H264AdvanceBits ARGS1(table[code][0]);//flushbits(val&0xFF);
	return table[code][1];
#else
	static const huf_decod_node_t *table = (huf_decod_node_t *) vlctab;

	code = H264ShowBits ARGS1(8);//PeekBits(8);
	H264AdvanceBits ARGS1(table[code].bits_num);//flushbits(val&0xFF);
	return table[code].value;
#endif //_ARM_FIX_
#else
	code = vlctab[H264ShowBits ARGS1(8)];
	H264AdvanceBits ARGS1(code&0xFF);//flushbits(val&0xFF);
	return code>>8;
#endif

}


/*!
************************************************************************
* \brief
*    read Level VLC codeword from UVLC-partition 
************************************************************************
*/
int readSyntaxElement_Level_VLC PARGS1(int vlcnum)
{
#if 1
	int len, sign, level, code;

	// read pre zeros
	len = find_leading_1 ARGS0();

	if(vlcnum==0)
	{// Previously readSyntaxElement_Level_VLC0 code
		if (len < 14)
		{
			sign  = -(len & 1);
			level = (len >> 1) + 1;
		}
		else if (len == 14)
		{
			// escape code
			code  = H264GetBits ARGS1(4);
			sign  = -(code & 1);
			level = (code >> 1) + 8;
		}
		else
		{
			// escape code
			const int escape =  16 - 2048;
			code  = H264GetBits ARGS1(len-3);
			sign  = -(code & 1);
			level = (code >> 1) + (1<<(len-4)) + escape;
		}
	}
	else
	{ // Previously readSyntaxElement_Level_VLCN code
		int shift = vlcnum-1;

		if (len < 15)
		{
			if (shift==0)
			{
				level = len + 1;
				// read 1 bit -> sign
				sign = -(int) H264GetBits ARGS1(1);
			}
			else
			{
				// read (shift) bits -> suffix
				const int escape = (len<<shift) + 1;

				code  = H264GetBits ARGS1(vlcnum);
				sign  = -(code & 1);
				level = (code >> 1) + escape;
			}
		}
		else // escape
		{
			const int escape = (15<<shift) + 1 - 2048;

			code  = H264GetBits ARGS1(len-3);
			sign  = -(code & 1);
			level = (code >> 1) + (1<<(len-4)) + escape;
		}

	}

	return level - ((level<<1)&sign);

#else

	// Lookup table, [vlcnum][len]
	static const short  __declspec(align(16)) read_add_amnt_0[20][2] GCC_ALIGN(16) = {
		{ 0,     2+( 0<<0)},{ 0,     2+( 1<<0)},{ 0,     2+( 2<<0)},{ 0,     2+( 3<<0)},
		{ 0,     2+( 4<<0)},{ 0,     2+( 5<<0)},{ 0,     2+( 6<<0)},{ 0,     2+( 7<<0)},
		{ 0,     2+( 8<<0)},{ 0,     2+( 9<<0)},{ 0,     2+(10<<0)},{ 0,     2+(11<<0)},
		{ 0,     2+(12<<0)},{ 0,     2+(13<<0)},{ 4,     2+(14<<0)},{12,            32},
		{13, 32+(4096<< 0)},{14, 32+(4096<< 1)},{15, 32+(4096<< 2)},{16, 32+(4096<< 3)}
	};
	static const short  read_add_amnt_1[20][2] = {
		{ 1,     2+( 0<<1)},{ 1,     2+( 1<<1)},{ 1,     2+( 2<<1)},{ 1,     2+( 3<<1)},
		{ 1,     2+( 4<<1)},{ 1,     2+( 5<<1)},{ 1,     2+( 6<<1)},{ 1,     2+( 7<<1)},
		{ 1,     2+( 8<<1)},{ 1,     2+( 9<<1)},{ 1,     2+(10<<1)},{ 1,     2+(11<<1)},
		{ 1,     2+(12<<1)},{ 1,     2+(13<<1)},{ 1,     2+(14<<1)},{12,     2+(15<<1)},
		{13, 32+(4096<< 0)},{14, 32+(4096<< 1)},{15, 32+(4096<< 2)},{16, 32+(4096<< 3)}
	};
	static const short  read_add_amnt_2[20][2] = {
		{ 2,     2+( 0<<2)},{ 2,     2+( 1<<2)},{ 2,     2+( 2<<2)},{ 2,     2+( 3<<2)},
		{ 2,     2+( 4<<2)},{ 2,     2+( 5<<2)},{ 2,     2+( 6<<2)},{ 2,     2+( 7<<2)},
		{ 2,     2+( 8<<2)},{ 2,     2+( 9<<2)},{ 2,     2+(10<<2)},{ 2,     2+(11<<2)},
		{ 2,     2+(12<<2)},{ 2,     2+(13<<2)},{ 2,     2+(14<<2)},{12,     2+(15<<2)},
		{13, 62+(4096<< 0)},{14, 62+(4096<< 1)},{15, 62+(4096<< 2)},{16, 62+(4096<< 3)}
	};
	static const short  read_add_amnt_3[20][2] = {
		{ 3,     2+( 0<<3)},{ 3,     2+( 1<<3)},{ 3,     2+( 2<<3)},{ 3,     2+( 3<<3)},
		{ 3,     2+( 4<<3)},{ 3,     2+( 5<<3)},{ 3,     2+( 6<<3)},{ 3,     2+( 7<<3)},
		{ 3,     2+( 8<<3)},{ 3,     2+( 9<<3)},{ 3,     2+(10<<3)},{ 3,     2+(11<<3)},
		{ 3,     2+(12<<3)},{ 3,     2+(13<<3)},{ 3,     2+(14<<3)},{12,     2+(15<<3)},
		{13,122+(4096<< 0)},{14,122+(4096<< 1)},{15,122+(4096<< 2)},{16,122+(4096<< 3)}
	};
	static const short  read_add_amnt_4[20][2] = {
		{ 4,     2+( 0<<4)},{ 4,     2+( 1<<4)},{ 4,     2+( 2<<4)},{ 4,     2+( 3<<4)},
		{ 4,     2+( 4<<4)},{ 4,     2+( 5<<4)},{ 4,     2+( 6<<4)},{ 4,     2+( 7<<4)},
		{ 4,     2+( 8<<4)},{ 4,     2+( 9<<4)},{ 4,     2+(10<<4)},{ 4,     2+(11<<4)},
		{ 4,     2+(12<<4)},{ 4,     2+(13<<4)},{ 4,     2+(14<<4)},{12,     2+(15<<4)},
		{13,242+(4096<< 0)},{14,242+(4096<< 1)},{15,242+(4096<< 2)},{16,242+(4096<< 3)}
	};
	static const short  read_add_amnt_5[20][2] = {
		{ 5,     2+( 0<<5)},{ 5,     2+( 1<<5)},{ 5,     2+( 2<<5)},{ 5,     2+( 3<<5)},
		{ 5,     2+( 4<<5)},{ 5,     2+( 5<<5)},{ 5,     2+( 6<<5)},{ 5,     2+( 7<<5)},
		{ 5,     2+( 8<<5)},{ 5,     2+( 9<<5)},{ 5,     2+(10<<5)},{ 5,     2+(11<<5)},
		{ 5,     2+(12<<5)},{ 5,     2+(13<<5)},{ 5,     2+(14<<5)},{12,     2+(15<<5)},
		{13,482+(4096<< 0)},{14,482+(4096<< 1)},{15,482+(4096<< 2)},{16,482+(4096<< 3)}
	};
	static const short  read_add_amnt_6[20][2] = {
		{ 6,     2+( 0<<6)},{ 6,     2+( 1<<6)},{ 6,     2+( 2<<6)},{ 6,     2+( 3<<6)},
		{ 6,     2+( 4<<6)},{ 6,     2+( 5<<6)},{ 6,     2+( 6<<6)},{ 6,     2+( 7<<6)},
		{ 6,     2+( 8<<6)},{ 6,     2+( 9<<6)},{ 6,     2+(10<<6)},{ 6,     2+(11<<6)},
		{ 6,     2+(12<<6)},{ 6,     2+(13<<6)},{ 6,     2+(14<<6)},{12,     2+(15<<6)},
		{13,962+(4096<< 0)},{14,962+(4096<< 1)},{15,962+(4096<< 2)},{16,962+(4096<< 3)}
	};

	static const short *read_add_amnt[] = { 
		(short *) read_add_amnt_0, (short *) read_add_amnt_1,
		(short *) read_add_amnt_2, (short *) read_add_amnt_3,
		(short *) read_add_amnt_4, (short *) read_add_amnt_5,
		(short *) read_add_amnt_6
	};

	short *table = (short *) read_add_amnt[vlcnum];

	int len,code, sign, level;
	// read pre zeros
	len    = find_leading_1 ARGS0();
	code   = table[2*len];
	if(code)
		code = H264GetBits ARGS1(code);
	code  += table[2*len+1];
	sign   = -(code & 1);
	level  = (code >> 1);
	return level - ((level<<1)&sign);

#endif

}

/*!
************************************************************************
* \brief
*    read Total Zeros codeword from UVLC-partition 
************************************************************************
*/
int readSyntaxElement_TotalZeros PARGS1(int vlcnum)
{
#if 0
	int code;

	int len;
	short *tab;	

	static const unsigned short total_zeros_0[] =
	{
		_C1L(1,0),  _C1L(4,0),  _C1L(4,4),  _C1L(3,4),  
		_C1L(2,3),  _C1L(2,3),  _C1L(1,3),  _C1L(1,3),
		_C1L(0,1),	_C1L(0,1),  _C1L(0,1),  _C1L(0,1),  
		_C1L(0,1),  _C1L(0,1),  _C1L(0,1),  _C1L(0,1),

		_C1L(2,0),  _C1L(3,0),  _C1L(12,4), _C1L(11,4), 
		_C1L(10,3), _C1L(10,3), _C1L(9,3),  _C1L(9,3),
		_C1L(8,2),  _C1L(8,2),  _C1L(8,2),  _C1L(8,2),  
		_C1L(7,2),  _C1L(7,2),  _C1L(7,2),  _C1L(7,2),

		_C1L(255,4),_C1L(255,4),_C1L(255,4),_C1L(255,4),
		_C1L(255,4),_C1L(255,4),_C1L(255,4),_C1L(255,4),
		_C1L(15,1), _C1L(15,1), _C1L(15,1), _C1L(15,1), 
		_C1L(15,1), _C1L(15,1), _C1L(15,1), _C1L(15,1), 

		_C1L(14,1), _C1L(14,1), _C1L(14,1), _C1L(14,1), 
		_C1L(14,1), _C1L(14,1), _C1L(14,1), _C1L(14,1), 
		_C1L(13,1), _C1L(13,1), _C1L(13,1), _C1L(13,1), 
		_C1L(13,1), _C1L(13,1), _C1L(13,1), _C1L(13,1),

		_C1L(6,1),  _C1L(6,1),  _C1L(6,1),  _C1L(6,1),  
		_C1L(6,1),  _C1L(6,1),  _C1L(6,1),  _C1L(6,1),  
		_C1L(5,1),  _C1L(5,1),  _C1L(5,1),  _C1L(5,1),  
		_C1L(5,1),  _C1L(5,1),  _C1L(5,1),  _C1L(5,1),
	};

	//1-16
	//table=4bits and 2bits
	static const unsigned short total_zeros[14][24] = {	
		{
			_C1L(0,0),  _C1L(1,0),  _C1L(8,4),  _C1L(7,4),  
				_C1L(6,4),  _C1L(5,4),  _C1L(4,3),  _C1L(4,3),
				_C1L(3,3),  _C1L(3,3),  _C1L(2,3),  _C1L(2,3),  
				_C1L(1,3),  _C1L(1,3),  _C1L(0,3),  _C1L(0,3),

				_C1L(14,2), _C1L(13,2), _C1L(12,2), _C1L(11,2), 		
				_C1L(10,1), _C1L(10,1), _C1L(9,1),  _C1L(9,1)
		},

		{
			_C1L(0,0),  _C1L(1,0),  _C1L(8,4),  _C1L(5,4),  
				_C1L(4,4),  _C1L(0,4),  _C1L(7,3),  _C1L(7,3),
				_C1L(6,3),  _C1L(6,3),  _C1L(3,3),  _C1L(3,3),  
				_C1L(2,3),  _C1L(2,3),  _C1L(1,3),  _C1L(1,3),

				_C1L(13,2), _C1L(11,2), _C1L(12,1), _C1L(12,1),
				_C1L(10,1), _C1L(10,1), _C1L(9,1),  _C1L(9,1)
			},

			{
				_C1L(0,0),  _C1L(1,0),  _C1L(9,4),  _C1L(7,4),  
					_C1L(3,4),  _C1L(2,4),  _C1L(8,3),  _C1L(8,3),
					_C1L(6,3),  _C1L(6,3),  _C1L(5,3),  _C1L(5,3),  
					_C1L(4,3),  _C1L(4,3),  _C1L(1,3),  _C1L(1,3),

					_C1L(12,1), _C1L(12,1), _C1L(11,1), _C1L(11,1),
					_C1L(10,1), _C1L(10,1), _C1L(0,1),  _C1L(0,1)
			},

			{
				_C1L(0,0),  _C1L(10,4), _C1L(8,4),  _C1L(2,4),  
					_C1L(1,4),  _C1L(0,4),  _C1L(7,3),  _C1L(7,3),
					_C1L(6,3),  _C1L(6,3),  _C1L(5,3),  _C1L(5,3),  
					_C1L(4,3),  _C1L(4,3),  _C1L(3,3),  _C1L(3,3),

					_C1L(11,1), _C1L(11,1), _C1L(9,1),  _C1L(9,1)
				},

				{
					_C1L(0,0),  _C1L(8,4),  _C1L(9,3),  _C1L(9,3),  
						_C1L(7,3),  _C1L(7,3),  _C1L(6,3),  _C1L(6,3),
						_C1L(5,3),  _C1L(5,3),  _C1L(4,3),  _C1L(4,3),  
						_C1L(3,3),  _C1L(3,3),  _C1L(2,3),  _C1L(2,3),

						_C1L(10,2), _C1L(0,2),  _C1L(1,1),  _C1L(1,1)
				},

				{
					_C1L(0,0),  _C1L(7,4),  _C1L(8,3),  _C1L(8,3),  
						_C1L(6,3),  _C1L(6,3),  _C1L(4,3),  _C1L(4,3),
						_C1L(3,3),  _C1L(3,3),  _C1L(2,3),  _C1L(2,3),  
						_C1L(5,2),  _C1L(5,2),  _C1L(5,2),  _C1L(5,2),

						_C1L(9,2),  _C1L(0,2),  _C1L(1,1),  _C1L(1,1)
					},

					{
						_C1L(0,0), _C1L(1,4), _C1L(7,3), _C1L(7,3), 
							_C1L(6,3), _C1L(6,3), _C1L(3,3), _C1L(3,3),
							_C1L(5,2), _C1L(5,2), _C1L(5,2), _C1L(5,2), 
							_C1L(4,2), _C1L(4,2), _C1L(4,2), _C1L(4,2),

							_C1L(8,2), _C1L(0,2), _C1L(2,1), _C1L(2,1)
					},

					{
						_C1L(0,0), _C1L(2,4), _C1L(5,3), _C1L(5,3), 
							_C1L(6,2), _C1L(6,2), _C1L(6,2), _C1L(6,2),
							_C1L(4,2), _C1L(4,2), _C1L(4,2), _C1L(4,2), 
							_C1L(3,2), _C1L(3,2), _C1L(3,2), _C1L(3,2),

							_C1L(1,2), _C1L(0,2), _C1L(7,1), _C1L(7,1)
						},

						{
							_C1L(0,0), _C1L(6,4), _C1L(2,3), _C1L(2,3), 
								_C1L(5,2), _C1L(5,2), _C1L(5,2), _C1L(5,2),
								_C1L(4,2), _C1L(4,2), _C1L(4,2), _C1L(4,2), 
								_C1L(3,2), _C1L(3,2), _C1L(3,2), _C1L(3,2),

								_C1L(1,1), _C1L(1,1), _C1L(0,1), _C1L(0,1)
						},

						{
							_C1L(0,4), _C1L(1,4), _C1L(2,3), _C1L(2,3), 
								_C1L(3,3), _C1L(3,3), _C1L(5,3), _C1L(5,3),
								_C1L(4,1), _C1L(4,1), _C1L(4,1), _C1L(4,1), 
								_C1L(4,1), _C1L(4,1), _C1L(4,1), _C1L(4,1) 
							},

							{
								_C1L(0,4), _C1L(1,4), _C1L(4,3), _C1L(4,3), 
									_C1L(2,2), _C1L(2,2), _C1L(2,2), _C1L(2,2),
									_C1L(3,1), _C1L(3,1), _C1L(3,1), _C1L(3,1), 
									_C1L(3,1), _C1L(3,1), _C1L(3,1), _C1L(3,1) 
							},

							{
								_C1L(0,3), _C1L(0,3), _C1L(1,3), _C1L(1,3), 
									_C1L(3,2), _C1L(3,2), _C1L(3,2), _C1L(3,2),
									_C1L(2,1), _C1L(2,1), _C1L(2,1), _C1L(2,1), 
									_C1L(2,1), _C1L(2,1), _C1L(2,1), _C1L(2,1) 
								},

								{
									_C1L(0,2), _C1L(0,2), _C1L(0,2), _C1L(0,2), 
										_C1L(1,2), _C1L(1,2), _C1L(1,2), _C1L(1,2),
										_C1L(2,1), _C1L(2,1), _C1L(2,1), _C1L(2,1), 
										_C1L(2,1), _C1L(2,1), _C1L(2,1), _C1L(2,1) 
								},

								{
									_C1L(0,1), _C1L(0,1), _C1L(0,1), _C1L(0,1), 
										_C1L(0,1), _C1L(0,1), _C1L(0,1), _C1L(0,1), 
										_C1L(1,1), _C1L(1,1), _C1L(1,1), _C1L(1,1), 
										_C1L(1,1), _C1L(1,1), _C1L(1,1), _C1L(1,1)
									}
	};

	if(vlcnum>0)
	{
		tab  = (short*)total_zeros[vlcnum-1];		
		code = tab[H264ShowBits ARGS1(4)];
		if((len=(code&0xFF))==0)
		{						
			H264AdvanceBits ARGS1(4);
			tab += 16+((code>>8)<<2);
			code = tab[H264ShowBits ARGS1(2)];
			len  = code&0xFF;
		}
	}
	else
	{
		code = total_zeros_0[H264ShowBits ARGS1(4)];
		while((len=(code&0xFF))==0)
		{
			H264AdvanceBits ARGS1(4);
			code = total_zeros_0[H264ShowBits ARGS1(4)+((code>>8)<<4)];
		}
	}
	H264AdvanceBits ARGS1(len);

	return code>>8;

#elif 1

	int code;
	int bits_num;

	//table - 6 bits + 5 bits (for 1)
	static const unsigned short total_zeros[16][64] =
	{
		// TotalCoeff(coeff_token=1)
		{
			_C1L( 0,0), _C1L( 0,0), _C1L( 8,6), _C1L( 7,6),
				_C1L( 6,5), _C1L( 6,5), _C1L( 5,5), _C1L( 5,5),
				_C1L( 4,4),	_C1L( 4,4), _C1L( 4,4), _C1L( 4,4),
				_C1L( 3,4), _C1L( 3,4), _C1L( 3,4), _C1L( 3,4),

				_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
				_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
				_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3),
				_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3),

				_C1L( 0,1), _C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
				_C1L( 0,1), _C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
				_C1L( 0,1), _C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
				_C1L( 0,1), _C1L( 0,1), _C1L( 0,1), _C1L( 0,1),

				_C1L( 0,1), _C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
				_C1L( 0,1), _C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
				_C1L( 0,1), _C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
				_C1L( 0,1), _C1L( 0,1), _C1L( 0,1), _C1L( 0,1)
		},
		// TotalCoeff(coeff_token=2)
		{
			_C1L(14,6), _C1L(13,6), _C1L(12,6), _C1L(11,6),
				_C1L(10,5), _C1L(10,5), _C1L( 9,5), _C1L( 9,5),
				_C1L( 8,4),	_C1L( 8,4), _C1L( 8,4), _C1L( 8,4),
				_C1L( 7,4), _C1L( 7,4), _C1L( 7,4), _C1L( 7,4),

				_C1L( 6,4), _C1L( 6,4), _C1L( 6,4), _C1L( 6,4),
				_C1L( 5,4), _C1L( 5,4), _C1L( 5,4), _C1L( 5,4),
				_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),
				_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),

				_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
				_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
				_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
				_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),

				_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3),
				_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3),
				_C1L( 0,3), _C1L( 0,3), _C1L( 0,3), _C1L( 0,3),
				_C1L( 0,3), _C1L( 0,3), _C1L( 0,3), _C1L( 0,3)
			},
			// TotalCoeff(coeff_token=3)
			{
				_C1L(13,6), _C1L(11,6), _C1L(12,5), _C1L(12,5),
					_C1L(10,5), _C1L(10,5), _C1L( 9,5), _C1L( 9,5),
					_C1L( 8,4),	_C1L( 8,4), _C1L( 8,4), _C1L( 8,4),
					_C1L( 5,4), _C1L( 5,4), _C1L( 5,4), _C1L( 5,4),

					_C1L( 4,4), _C1L( 4,4), _C1L( 4,4), _C1L( 4,4),
					_C1L( 0,4), _C1L( 0,4), _C1L( 0,4), _C1L( 0,4),
					_C1L( 7,3), _C1L( 7,3), _C1L( 7,3), _C1L( 7,3),
					_C1L( 7,3), _C1L( 7,3), _C1L( 7,3), _C1L( 7,3),

					_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
					_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
					_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
					_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),

					_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
					_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
					_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3),
					_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3)
			},
			// TotalCoeff(coeff_token=4)
			{
				_C1L(12,5), _C1L(12,5), _C1L(11,5), _C1L(11,5),
					_C1L(10,5), _C1L(10,5), _C1L( 0,5), _C1L( 0,5),
					_C1L( 9,4),	_C1L( 9,4), _C1L( 9,4), _C1L( 9,4),
					_C1L( 7,4), _C1L( 7,4), _C1L( 7,4), _C1L( 7,4),

					_C1L( 3,4), _C1L( 3,4), _C1L( 3,4), _C1L( 3,4),
					_C1L( 2,4), _C1L( 2,4), _C1L( 2,4), _C1L( 2,4),
					_C1L( 8,3), _C1L( 8,3), _C1L( 8,3), _C1L( 8,3),
					_C1L( 8,3), _C1L( 8,3), _C1L( 8,3), _C1L( 8,3),

					_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
					_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
					_C1L( 5,3), _C1L( 5,3), _C1L( 5,3), _C1L( 5,3),
					_C1L( 5,3), _C1L( 5,3), _C1L( 5,3), _C1L( 5,3),

					_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),
					_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),
					_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3),
					_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3)
				},
				// TotalCoeff(coeff_token=5)
				{
					_C1L(11,5), _C1L(11,5), _C1L( 9,5), _C1L( 9,5),
						_C1L(10,4), _C1L(10,4), _C1L(10,4), _C1L(10,4),
						_C1L( 8,4),	_C1L( 8,4), _C1L( 8,4), _C1L( 8,4),
						_C1L( 2,4), _C1L( 2,4), _C1L( 2,4), _C1L( 2,4),

						_C1L( 1,4), _C1L( 1,4), _C1L( 1,4), _C1L( 1,4),
						_C1L( 0,4), _C1L( 0,4), _C1L( 0,4), _C1L( 0,4),
						_C1L( 7,3), _C1L( 7,3), _C1L( 7,3), _C1L( 7,3),
						_C1L( 7,3), _C1L( 7,3), _C1L( 7,3), _C1L( 7,3),

						_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
						_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
						_C1L( 5,3), _C1L( 5,3), _C1L( 5,3), _C1L( 5,3),
						_C1L( 5,3), _C1L( 5,3), _C1L( 5,3), _C1L( 5,3),

						_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),
						_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),
						_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
						_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3)
				},
				// TotalCoeff(coeff_token=6)
				{
					_C1L(10,6), _C1L( 0,6), _C1L( 1,5), _C1L( 1,5),
						_C1L( 8,4),	_C1L( 8,4), _C1L( 8,4), _C1L( 8,4),
						_C1L( 9,3),	_C1L( 9,3), _C1L( 9,3), _C1L( 9,3),
						_C1L( 9,3),	_C1L( 9,3), _C1L( 9,3), _C1L( 9,3),

						_C1L( 7,3), _C1L( 7,3), _C1L( 7,3), _C1L( 7,3),
						_C1L( 7,3), _C1L( 7,3), _C1L( 7,3), _C1L( 7,3),
						_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
						_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),

						_C1L( 5,3), _C1L( 5,3), _C1L( 5,3), _C1L( 5,3),
						_C1L( 5,3), _C1L( 5,3), _C1L( 5,3), _C1L( 5,3),
						_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),
						_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),

						_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
						_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
						_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
						_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3)
					},
					// TotalCoeff(coeff_token=7)
					{
						_C1L( 9,6), _C1L( 0,6), _C1L( 1,5), _C1L( 1,5),
							_C1L( 7,4),	_C1L( 7,4), _C1L( 7,4), _C1L( 7,4),
							_C1L( 8,3),	_C1L( 8,3), _C1L( 8,3), _C1L( 8,3),
							_C1L( 8,3),	_C1L( 8,3), _C1L( 8,3), _C1L( 8,3),

							_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
							_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
							_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),
							_C1L( 4,3), _C1L( 4,3), _C1L( 4,3), _C1L( 4,3),

							_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
							_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
							_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
							_C1L( 2,3), _C1L( 2,3), _C1L( 2,3), _C1L( 2,3),

							_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
							_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
							_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
							_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2)
					},
					// TotalCoeff(coeff_token=8)
					{
						_C1L( 8,6), _C1L( 0,6), _C1L( 2,5), _C1L( 2,5),
							_C1L( 1,4),	_C1L( 1,4), _C1L( 1,4), _C1L( 1,4),
							_C1L( 7,3),	_C1L( 7,3), _C1L( 7,3), _C1L( 7,3),
							_C1L( 7,3),	_C1L( 7,3), _C1L( 7,3), _C1L( 7,3),

							_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
							_C1L( 6,3), _C1L( 6,3), _C1L( 6,3), _C1L( 6,3),
							_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
							_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),

							_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
							_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
							_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
							_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),

							_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
							_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
							_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
							_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2)
						},
						// TotalCoeff(coeff_token=9)
						{
							_C1L( 1,6), _C1L( 0,6), _C1L( 7,5), _C1L( 7,5),
								_C1L( 2,4),	_C1L( 2,4), _C1L( 2,4), _C1L( 2,4),
								_C1L( 5,3),	_C1L( 5,3), _C1L( 5,3), _C1L( 5,3),
								_C1L( 5,3),	_C1L( 5,3), _C1L( 5,3), _C1L( 5,3),

								_C1L( 6,2), _C1L( 6,2), _C1L( 6,2), _C1L( 6,2),
								_C1L( 6,2), _C1L( 6,2), _C1L( 6,2), _C1L( 6,2),
								_C1L( 6,2), _C1L( 6,2), _C1L( 6,2), _C1L( 6,2),
								_C1L( 6,2), _C1L( 6,2), _C1L( 6,2), _C1L( 6,2),

								_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
								_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
								_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
								_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),

								_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
								_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
								_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
								_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2)
						},
						// TotalCoeff(coeff_token=10)
						{
							_C1L( 1,5), _C1L( 1,5), _C1L( 0,5), _C1L( 0,5),
								_C1L( 6,4),	_C1L( 6,4), _C1L( 6,4), _C1L( 6,4),
								_C1L( 2,3),	_C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
								_C1L( 2,3),	_C1L( 2,3), _C1L( 2,3), _C1L( 2,3),

								_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
								_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
								_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),
								_C1L( 5,2), _C1L( 5,2), _C1L( 5,2), _C1L( 5,2),

								_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
								_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
								_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),
								_C1L( 4,2), _C1L( 4,2), _C1L( 4,2), _C1L( 4,2),

								_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
								_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
								_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
								_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2)
							},
							// TotalCoeff(coeff_token=11)
							{
								_C1L( 0,4),	_C1L( 0,4), _C1L( 0,4), _C1L( 0,4),
									_C1L( 1,4), _C1L( 1,4), _C1L( 1,4), _C1L( 1,4),
									_C1L( 2,3),	_C1L( 2,3), _C1L( 2,3), _C1L( 2,3),
									_C1L( 2,3),	_C1L( 2,3), _C1L( 2,3), _C1L( 2,3),

									_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
									_C1L( 3,3), _C1L( 3,3), _C1L( 3,3), _C1L( 3,3),
									_C1L( 5,3), _C1L( 5,3), _C1L( 5,3), _C1L( 5,3),
									_C1L( 5,3), _C1L( 5,3), _C1L( 5,3), _C1L( 5,3),

									_C1L( 4,1), _C1L( 4,1), _C1L( 4,1), _C1L( 4,1),
									_C1L( 4,1), _C1L( 4,1), _C1L( 4,1), _C1L( 4,1),
									_C1L( 4,1), _C1L( 4,1), _C1L( 4,1), _C1L( 4,1),
									_C1L( 4,1), _C1L( 4,1), _C1L( 4,1), _C1L( 4,1),

									_C1L( 4,1), _C1L( 4,1), _C1L( 4,1), _C1L( 4,1),
									_C1L( 4,1), _C1L( 4,1), _C1L( 4,1), _C1L( 4,1),
									_C1L( 4,1), _C1L( 4,1), _C1L( 4,1), _C1L( 4,1),
									_C1L( 4,1), _C1L( 4,1), _C1L( 4,1), _C1L( 4,1)
							},
							// TotalCoeff(coeff_token=12)
							{
								_C1L( 0,4),	_C1L( 0,4), _C1L( 0,4), _C1L( 0,4),
									_C1L( 1,4), _C1L( 1,4), _C1L( 1,4), _C1L( 1,4),
									_C1L( 4,3),	_C1L( 4,3), _C1L( 4,3), _C1L( 4,3),
									_C1L( 4,3),	_C1L( 4,3), _C1L( 4,3), _C1L( 4,3),

									_C1L( 2,2), _C1L( 2,2), _C1L( 2,2), _C1L( 2,2),
									_C1L( 2,2), _C1L( 2,2), _C1L( 2,2), _C1L( 2,2),
									_C1L( 2,2), _C1L( 2,2), _C1L( 2,2), _C1L( 2,2),
									_C1L( 2,2), _C1L( 2,2), _C1L( 2,2), _C1L( 2,2),

									_C1L( 3,1), _C1L( 3,1), _C1L( 3,1), _C1L( 3,1),
									_C1L( 3,1), _C1L( 3,1), _C1L( 3,1), _C1L( 3,1),
									_C1L( 3,1), _C1L( 3,1), _C1L( 3,1), _C1L( 3,1),
									_C1L( 3,1), _C1L( 3,1), _C1L( 3,1), _C1L( 3,1),

									_C1L( 3,1), _C1L( 3,1), _C1L( 3,1), _C1L( 3,1),
									_C1L( 3,1), _C1L( 3,1), _C1L( 3,1), _C1L( 3,1),
									_C1L( 3,1), _C1L( 3,1), _C1L( 3,1), _C1L( 3,1),
									_C1L( 3,1), _C1L( 3,1), _C1L( 3,1), _C1L( 3,1)
								},
								// TotalCoeff(coeff_token=13)
								{
									_C1L( 0,3),	_C1L( 0,3), _C1L( 0,3), _C1L( 0,3),
										_C1L( 0,3),	_C1L( 0,3), _C1L( 0,3), _C1L( 0,3),
										_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3),
										_C1L( 1,3), _C1L( 1,3), _C1L( 1,3), _C1L( 1,3),

										_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
										_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
										_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),
										_C1L( 3,2), _C1L( 3,2), _C1L( 3,2), _C1L( 3,2),

										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),

										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1)
								},
								// TotalCoeff(coeff_token=14)
								{
									_C1L( 0,2),	_C1L( 0,2), _C1L( 0,2), _C1L( 0,2),
										_C1L( 0,2),	_C1L( 0,2), _C1L( 0,2), _C1L( 0,2),
										_C1L( 0,2),	_C1L( 0,2), _C1L( 0,2), _C1L( 0,2),
										_C1L( 0,2),	_C1L( 0,2), _C1L( 0,2), _C1L( 0,2),

										_C1L( 1,2), _C1L( 1,2), _C1L( 1,2), _C1L( 1,2),
										_C1L( 1,2), _C1L( 1,2), _C1L( 1,2), _C1L( 1,2),
										_C1L( 1,2), _C1L( 1,2), _C1L( 1,2), _C1L( 1,2),
										_C1L( 1,2), _C1L( 1,2), _C1L( 1,2), _C1L( 1,2),

										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),

										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1),
										_C1L( 2,1), _C1L( 2,1), _C1L( 2,1), _C1L( 2,1)
									},
									// TotalCoeff(coeff_token=15)
									{
										_C1L( 0,1),	_C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
											_C1L( 0,1),	_C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
											_C1L( 0,1),	_C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
											_C1L( 0,1),	_C1L( 0,1), _C1L( 0,1), _C1L( 0,1),

											_C1L( 0,1),	_C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
											_C1L( 0,1),	_C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
											_C1L( 0,1),	_C1L( 0,1), _C1L( 0,1), _C1L( 0,1),
											_C1L( 0,1),	_C1L( 0,1), _C1L( 0,1), _C1L( 0,1),

											_C1L( 1,1), _C1L( 1,1), _C1L( 1,1), _C1L( 1,1),
											_C1L( 1,1), _C1L( 1,1), _C1L( 1,1), _C1L( 1,1),
											_C1L( 1,1), _C1L( 1,1), _C1L( 1,1), _C1L( 1,1),
											_C1L( 1,1), _C1L( 1,1), _C1L( 1,1), _C1L( 1,1),

											_C1L( 1,1), _C1L( 1,1), _C1L( 1,1), _C1L( 1,1),
											_C1L( 1,1), _C1L( 1,1), _C1L( 1,1), _C1L( 1,1),
											_C1L( 1,1), _C1L( 1,1), _C1L( 1,1), _C1L( 1,1),
											_C1L( 1,1), _C1L( 1,1), _C1L( 1,1), _C1L( 1,1)
									},
									// TotalCoeff(coeff_token=1, next 3 bits)
									{
										// 0
										_C1L( 0,0),	_C1L(15,3), _C1L(14,3), _C1L(13,3),
											_C1L(12,2),	_C1L(12,2), _C1L(11,2), _C1L(11,2),
											// 1
											_C1L(10,1),	_C1L(10,1), _C1L(10,1), _C1L(10,1),
											_C1L( 9,1),	_C1L( 9,1), _C1L( 9,1), _C1L( 9,1),
										}
	};
#ifdef _ARM_FIX_
	unsigned char const (*table)[2] = reinterpret_cast<const unsigned char (*)[2]>(&total_zeros[vlcnum][0]);

	code = H264ShowBits ARGS1(6);
	bits_num = table[code][0];
	if(bits_num==0)
	{
		H264AdvanceBits ARGS1(6);
		table = reinterpret_cast<const unsigned char (*)[2]>(&total_zeros[15][code<<3]);
		code = H264ShowBits ARGS1(3);
		bits_num = table[code][0];
	}
	H264AdvanceBits ARGS1(bits_num);

	return table[code][1];
#else
	huf_decod_node_t *table = (huf_decod_node_t *) &total_zeros[vlcnum][0];

	code = H264ShowBits ARGS1(6);
	bits_num = table[code].bits_num;
	if(bits_num==0)
	{						
		H264AdvanceBits ARGS1(6);
		table = (huf_decod_node_t *) &total_zeros[15][code<<3];
		code = H264ShowBits ARGS1(3);
		bits_num = table[code].bits_num;
	}
	H264AdvanceBits ARGS1(bits_num);

	return table[code].value;
#endif //_ARM_FIX_
#else

	int code;
	int bits_num;

	static const byte total_zeros[16][64] =
	{
		// TotalCoeff(coeff_token=1)
		{
			_C0L( 0,0), _C0L( 0,0), _C0L( 8,6), _C0L( 7,6),
				_C0L( 6,5), _C0L( 6,5), _C0L( 5,5), _C0L( 5,5),
				_C0L( 4,4),	_C0L( 4,4), _C0L( 4,4), _C0L( 4,4),
				_C0L( 3,4), _C0L( 3,4), _C0L( 3,4), _C0L( 3,4),

				_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
				_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
				_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3),
				_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3),

				_C0L( 0,1), _C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
				_C0L( 0,1), _C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
				_C0L( 0,1), _C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
				_C0L( 0,1), _C0L( 0,1), _C0L( 0,1), _C0L( 0,1),

				_C0L( 0,1), _C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
				_C0L( 0,1), _C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
				_C0L( 0,1), _C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
				_C0L( 0,1), _C0L( 0,1), _C0L( 0,1), _C0L( 0,1)
		},
		// TotalCoeff(coeff_token=2)
		{
			_C0L(14,6), _C0L(13,6), _C0L(12,6), _C0L(11,6),
				_C0L(10,5), _C0L(10,5), _C0L( 9,5), _C0L( 9,5),
				_C0L( 8,4),	_C0L( 8,4), _C0L( 8,4), _C0L( 8,4),
				_C0L( 7,4), _C0L( 7,4), _C0L( 7,4), _C0L( 7,4),

				_C0L( 6,4), _C0L( 6,4), _C0L( 6,4), _C0L( 6,4),
				_C0L( 5,4), _C0L( 5,4), _C0L( 5,4), _C0L( 5,4),
				_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),
				_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),

				_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
				_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
				_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
				_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),

				_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3),
				_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3),
				_C0L( 0,3), _C0L( 0,3), _C0L( 0,3), _C0L( 0,3),
				_C0L( 0,3), _C0L( 0,3), _C0L( 0,3), _C0L( 0,3)
			},
			// TotalCoeff(coeff_token=3)
			{
				_C0L(13,6), _C0L(11,6), _C0L(12,5), _C0L(12,5),
					_C0L(10,5), _C0L(10,5), _C0L( 9,5), _C0L( 9,5),
					_C0L( 8,4),	_C0L( 8,4), _C0L( 8,4), _C0L( 8,4),
					_C0L( 5,4), _C0L( 5,4), _C0L( 5,4), _C0L( 5,4),

					_C0L( 4,4), _C0L( 4,4), _C0L( 4,4), _C0L( 4,4),
					_C0L( 0,4), _C0L( 0,4), _C0L( 0,4), _C0L( 0,4),
					_C0L( 7,3), _C0L( 7,3), _C0L( 7,3), _C0L( 7,3),
					_C0L( 7,3), _C0L( 7,3), _C0L( 7,3), _C0L( 7,3),

					_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
					_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
					_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
					_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),

					_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
					_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
					_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3),
					_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3)
			},
			// TotalCoeff(coeff_token=4)
			{
				_C0L(12,5), _C0L(12,5), _C0L(11,5), _C0L(11,5),
					_C0L(10,5), _C0L(10,5), _C0L( 0,5), _C0L( 0,5),
					_C0L( 9,4),	_C0L( 9,4), _C0L( 9,4), _C0L( 9,4),
					_C0L( 7,4), _C0L( 7,4), _C0L( 7,4), _C0L( 7,4),

					_C0L( 3,4), _C0L( 3,4), _C0L( 3,4), _C0L( 3,4),
					_C0L( 2,4), _C0L( 2,4), _C0L( 2,4), _C0L( 2,4),
					_C0L( 8,3), _C0L( 8,3), _C0L( 8,3), _C0L( 8,3),
					_C0L( 8,3), _C0L( 8,3), _C0L( 8,3), _C0L( 8,3),

					_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
					_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
					_C0L( 5,3), _C0L( 5,3), _C0L( 5,3), _C0L( 5,3),
					_C0L( 5,3), _C0L( 5,3), _C0L( 5,3), _C0L( 5,3),

					_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),
					_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),
					_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3),
					_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3)
				},
				// TotalCoeff(coeff_token=5)
				{
					_C0L(11,5), _C0L(11,5), _C0L( 9,5), _C0L( 9,5),
						_C0L(10,4), _C0L(10,4), _C0L(10,4), _C0L(10,4),
						_C0L( 8,4),	_C0L( 8,4), _C0L( 8,4), _C0L( 8,4),
						_C0L( 2,4), _C0L( 2,4), _C0L( 2,4), _C0L( 2,4),

						_C0L( 1,4), _C0L( 1,4), _C0L( 1,4), _C0L( 1,4),
						_C0L( 0,4), _C0L( 0,4), _C0L( 0,4), _C0L( 0,4),
						_C0L( 7,3), _C0L( 7,3), _C0L( 7,3), _C0L( 7,3),
						_C0L( 7,3), _C0L( 7,3), _C0L( 7,3), _C0L( 7,3),

						_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
						_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
						_C0L( 5,3), _C0L( 5,3), _C0L( 5,3), _C0L( 5,3),
						_C0L( 5,3), _C0L( 5,3), _C0L( 5,3), _C0L( 5,3),

						_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),
						_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),
						_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
						_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3)
				},
				// TotalCoeff(coeff_token=6)
				{
					_C0L(10,6), _C0L( 0,6), _C0L( 1,5), _C0L( 1,5),
						_C0L( 8,4),	_C0L( 8,4), _C0L( 8,4), _C0L( 8,4),
						_C0L( 9,3),	_C0L( 9,3), _C0L( 9,3), _C0L( 9,3),
						_C0L( 9,3),	_C0L( 9,3), _C0L( 9,3), _C0L( 9,3),

						_C0L( 7,3), _C0L( 7,3), _C0L( 7,3), _C0L( 7,3),
						_C0L( 7,3), _C0L( 7,3), _C0L( 7,3), _C0L( 7,3),
						_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
						_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),

						_C0L( 5,3), _C0L( 5,3), _C0L( 5,3), _C0L( 5,3),
						_C0L( 5,3), _C0L( 5,3), _C0L( 5,3), _C0L( 5,3),
						_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),
						_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),

						_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
						_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
						_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
						_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3)
					},
					// TotalCoeff(coeff_token=7)
					{
						_C0L( 9,6), _C0L( 0,6), _C0L( 1,5), _C0L( 1,5),
							_C0L( 7,4),	_C0L( 7,4), _C0L( 7,4), _C0L( 7,4),
							_C0L( 8,3),	_C0L( 8,3), _C0L( 8,3), _C0L( 8,3),
							_C0L( 8,3),	_C0L( 8,3), _C0L( 8,3), _C0L( 8,3),

							_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
							_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
							_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),
							_C0L( 4,3), _C0L( 4,3), _C0L( 4,3), _C0L( 4,3),

							_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
							_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
							_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
							_C0L( 2,3), _C0L( 2,3), _C0L( 2,3), _C0L( 2,3),

							_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
							_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
							_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
							_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2)
					},
					// TotalCoeff(coeff_token=8)
					{
						_C0L( 8,6), _C0L( 0,6), _C0L( 2,5), _C0L( 2,5),
							_C0L( 1,4),	_C0L( 1,4), _C0L( 1,4), _C0L( 1,4),
							_C0L( 7,3),	_C0L( 7,3), _C0L( 7,3), _C0L( 7,3),
							_C0L( 7,3),	_C0L( 7,3), _C0L( 7,3), _C0L( 7,3),

							_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
							_C0L( 6,3), _C0L( 6,3), _C0L( 6,3), _C0L( 6,3),
							_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
							_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),

							_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
							_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
							_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
							_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),

							_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
							_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
							_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
							_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2)
						},
						// TotalCoeff(coeff_token=9)
						{
							_C0L( 1,6), _C0L( 0,6), _C0L( 7,5), _C0L( 7,5),
								_C0L( 2,4),	_C0L( 2,4), _C0L( 2,4), _C0L( 2,4),
								_C0L( 5,3),	_C0L( 5,3), _C0L( 5,3), _C0L( 5,3),
								_C0L( 5,3),	_C0L( 5,3), _C0L( 5,3), _C0L( 5,3),

								_C0L( 6,2), _C0L( 6,2), _C0L( 6,2), _C0L( 6,2),
								_C0L( 6,2), _C0L( 6,2), _C0L( 6,2), _C0L( 6,2),
								_C0L( 6,2), _C0L( 6,2), _C0L( 6,2), _C0L( 6,2),
								_C0L( 6,2), _C0L( 6,2), _C0L( 6,2), _C0L( 6,2),

								_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
								_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
								_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
								_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),

								_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
								_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
								_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
								_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2)
						},
						// TotalCoeff(coeff_token=10)
						{
							_C0L( 1,5), _C0L( 1,5), _C0L( 0,5), _C0L( 0,5),
								_C0L( 6,4),	_C0L( 6,4), _C0L( 6,4), _C0L( 6,4),
								_C0L( 2,3),	_C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
								_C0L( 2,3),	_C0L( 2,3), _C0L( 2,3), _C0L( 2,3),

								_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
								_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
								_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),
								_C0L( 5,2), _C0L( 5,2), _C0L( 5,2), _C0L( 5,2),

								_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
								_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
								_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),
								_C0L( 4,2), _C0L( 4,2), _C0L( 4,2), _C0L( 4,2),

								_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
								_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
								_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
								_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2)
							},
							// TotalCoeff(coeff_token=11)
							{
								_C0L( 0,4),	_C0L( 0,4), _C0L( 0,4), _C0L( 0,4),
									_C0L( 1,4), _C0L( 1,4), _C0L( 1,4), _C0L( 1,4),
									_C0L( 2,3),	_C0L( 2,3), _C0L( 2,3), _C0L( 2,3),
									_C0L( 2,3),	_C0L( 2,3), _C0L( 2,3), _C0L( 2,3),

									_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
									_C0L( 3,3), _C0L( 3,3), _C0L( 3,3), _C0L( 3,3),
									_C0L( 5,3), _C0L( 5,3), _C0L( 5,3), _C0L( 5,3),
									_C0L( 5,3), _C0L( 5,3), _C0L( 5,3), _C0L( 5,3),

									_C0L( 4,1), _C0L( 4,1), _C0L( 4,1), _C0L( 4,1),
									_C0L( 4,1), _C0L( 4,1), _C0L( 4,1), _C0L( 4,1),
									_C0L( 4,1), _C0L( 4,1), _C0L( 4,1), _C0L( 4,1),
									_C0L( 4,1), _C0L( 4,1), _C0L( 4,1), _C0L( 4,1),

									_C0L( 4,1), _C0L( 4,1), _C0L( 4,1), _C0L( 4,1),
									_C0L( 4,1), _C0L( 4,1), _C0L( 4,1), _C0L( 4,1),
									_C0L( 4,1), _C0L( 4,1), _C0L( 4,1), _C0L( 4,1),
									_C0L( 4,1), _C0L( 4,1), _C0L( 4,1), _C0L( 4,1)
							},
							// TotalCoeff(coeff_token=12)
							{
								_C0L( 0,4),	_C0L( 0,4), _C0L( 0,4), _C0L( 0,4),
									_C0L( 1,4), _C0L( 1,4), _C0L( 1,4), _C0L( 1,4),
									_C0L( 4,3),	_C0L( 4,3), _C0L( 4,3), _C0L( 4,3),
									_C0L( 4,3),	_C0L( 4,3), _C0L( 4,3), _C0L( 4,3),

									_C0L( 2,2), _C0L( 2,2), _C0L( 2,2), _C0L( 2,2),
									_C0L( 2,2), _C0L( 2,2), _C0L( 2,2), _C0L( 2,2),
									_C0L( 2,2), _C0L( 2,2), _C0L( 2,2), _C0L( 2,2),
									_C0L( 2,2), _C0L( 2,2), _C0L( 2,2), _C0L( 2,2),

									_C0L( 3,1), _C0L( 3,1), _C0L( 3,1), _C0L( 3,1),
									_C0L( 3,1), _C0L( 3,1), _C0L( 3,1), _C0L( 3,1),
									_C0L( 3,1), _C0L( 3,1), _C0L( 3,1), _C0L( 3,1),
									_C0L( 3,1), _C0L( 3,1), _C0L( 3,1), _C0L( 3,1),

									_C0L( 3,1), _C0L( 3,1), _C0L( 3,1), _C0L( 3,1),
									_C0L( 3,1), _C0L( 3,1), _C0L( 3,1), _C0L( 3,1),
									_C0L( 3,1), _C0L( 3,1), _C0L( 3,1), _C0L( 3,1),
									_C0L( 3,1), _C0L( 3,1), _C0L( 3,1), _C0L( 3,1)
								},
								// TotalCoeff(coeff_token=13)
								{
									_C0L( 0,3),	_C0L( 0,3), _C0L( 0,3), _C0L( 0,3),
										_C0L( 0,3),	_C0L( 0,3), _C0L( 0,3), _C0L( 0,3),
										_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3),
										_C0L( 1,3), _C0L( 1,3), _C0L( 1,3), _C0L( 1,3),

										_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
										_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
										_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),
										_C0L( 3,2), _C0L( 3,2), _C0L( 3,2), _C0L( 3,2),

										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),

										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1)
								},
								// TotalCoeff(coeff_token=14)
								{
									_C0L( 0,2),	_C0L( 0,2), _C0L( 0,2), _C0L( 0,2),
										_C0L( 0,2),	_C0L( 0,2), _C0L( 0,2), _C0L( 0,2),
										_C0L( 0,2),	_C0L( 0,2), _C0L( 0,2), _C0L( 0,2),
										_C0L( 0,2),	_C0L( 0,2), _C0L( 0,2), _C0L( 0,2),

										_C0L( 1,2), _C0L( 1,2), _C0L( 1,2), _C0L( 1,2),
										_C0L( 1,2), _C0L( 1,2), _C0L( 1,2), _C0L( 1,2),
										_C0L( 1,2), _C0L( 1,2), _C0L( 1,2), _C0L( 1,2),
										_C0L( 1,2), _C0L( 1,2), _C0L( 1,2), _C0L( 1,2),

										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),

										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1),
										_C0L( 2,1), _C0L( 2,1), _C0L( 2,1), _C0L( 2,1)
									},
									// TotalCoeff(coeff_token=15)
									{
										_C0L( 0,1),	_C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
											_C0L( 0,1),	_C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
											_C0L( 0,1),	_C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
											_C0L( 0,1),	_C0L( 0,1), _C0L( 0,1), _C0L( 0,1),

											_C0L( 0,1),	_C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
											_C0L( 0,1),	_C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
											_C0L( 0,1),	_C0L( 0,1), _C0L( 0,1), _C0L( 0,1),
											_C0L( 0,1),	_C0L( 0,1), _C0L( 0,1), _C0L( 0,1),

											_C0L( 1,1), _C0L( 1,1), _C0L( 1,1), _C0L( 1,1),
											_C0L( 1,1), _C0L( 1,1), _C0L( 1,1), _C0L( 1,1),
											_C0L( 1,1), _C0L( 1,1), _C0L( 1,1), _C0L( 1,1),
											_C0L( 1,1), _C0L( 1,1), _C0L( 1,1), _C0L( 1,1),

											_C0L( 1,1), _C0L( 1,1), _C0L( 1,1), _C0L( 1,1),
											_C0L( 1,1), _C0L( 1,1), _C0L( 1,1), _C0L( 1,1),
											_C0L( 1,1), _C0L( 1,1), _C0L( 1,1), _C0L( 1,1),
											_C0L( 1,1), _C0L( 1,1), _C0L( 1,1), _C0L( 1,1)
									},
									// TotalCoeff(coeff_token=1, next 3 bits)
									{
										// 0
										_C0L( 0,0),	_C0L(15,3), _C0L(14,3), _C0L(13,3),
											_C0L(12,2),	_C0L(12,2), _C0L(11,2), _C0L(11,2),
											// 1
											_C0L(10,1),	_C0L(10,1), _C0L(10,1), _C0L(10,1),
											_C0L( 9,1),	_C0L( 9,1), _C0L( 9,1), _C0L( 9,1),
										}
	};
	byte *table = (byte *) &total_zeros[vlcnum][0];

	code = H264ShowBits ARGS1(6);
	bits_num = table[code];
	if(bits_num==0)
	{
		H264AdvanceBits ARGS1(6);
		bits_num = total_zeros[15][(code<<3)+H264ShowBits ARGS1(3)];
	}
	H264AdvanceBits ARGS1(bits_num&0x0F);

	return bits_num>>4;

#endif

}


/*!
************************************************************************
* \brief
*    read Total Zeros Chroma DC codeword from UVLC-partition 
************************************************************************
*/
int readSyntaxElement_TotalZerosChromaDC PARGS1(int vlcnum)
{
#if 1
	static const byte vlctab[3][8] = 
	{
		{
			_C0L(3,3),_C0L(2,3),
				_C0L(1,2),_C0L(1,2),
				_C0L(0,1),_C0L(0,1),
				_C0L(0,1),_C0L(0,1)
		},
		{
			_C0L(2,2),_C0L(1,2),
				_C0L(0,1),_C0L(0,1),
				_C0L(0,0),_C0L(0,0),
				_C0L(0,0),_C0L(0,0)
			},
			{
				_C0L(1,1),_C0L(0,1),
					_C0L(0,0),_C0L(0,0),
					_C0L(0,0),_C0L(0,0),
					_C0L(0,0),_C0L(0,0)
			}
	};

	int code;

	code  = vlctab[vlcnum][H264ShowBits ARGS1(3-vlcnum)];//vlctab[vlcnum][PeekBits(3-vlcnum)];
	H264AdvanceBits ARGS1(code&0x0F);

	return code>>4;

#else
	static const unsigned short vlctab[3][8] = 
	{
		{
			_C1L(3,3),_C1L(2,3),
				_C1L(1,2),_C1L(1,2),	
				_C1L(0,1),_C1L(0,1),_C1L(0,1),_C1L(0,1)
		},
		{	  
			_C1L(2,2),_C1L(1,2),	
				_C1L(0,1),_C1L(0,1),
				_C1L(0,0),_C1L(0,0),_C1L(0,0),_C1L(0,0),	
			},
			{	  
				_C1L(1,1),_C1L(0,1),
					_C1L(0,0),_C1L(0,0),_C1L(0,0),_C1L(0,0),_C1L(0,0),_C1L(0,0)	
			}
	};

#if 1
	int code;

	code  = vlctab[vlcnum][H264ShowBits ARGS1(3-vlcnum)];//vlctab[vlcnum][PeekBits(3-vlcnum)];
	H264AdvanceBits ARGS1(code&0xFF);

	return code>>8;

#else
#ifdef _ARM_FIX_
	unsigned char code[2];

	*((short *)code) = vlctab[vlcnum][H264ShowBits ARGS1(3-vlcnum)];
	H264AdvanceBits ARGS1(code[0]);

	return code[1];
#else
	huf_decod_node_t code;

	code.comb = vlctab[vlcnum][H264ShowBits ARGS1(3-vlcnum)];
	H264AdvanceBits ARGS1(code.bits_num);

	return code.value;
#endif //_ARM_FIX_
#endif
#endif
}


/*!
************************************************************************
* \brief
*    read  Run codeword from UVLC-partition 
************************************************************************
*/
int readSyntaxElement_Run PARGS1(int vlcnum)
{
	int retval;
	int code;

	//zero left
	static const byte run_before[7][8] = 
	{
		//1
		{
			_C0L(1,1),_C0L(1,1),
				_C0L(1,1),_C0L(1,1),
				_C0L(0,1),_C0L(0,1),
				_C0L(0,1),_C0L(0,1)  
		},
		//2
		{
			_C0L(2,2),_C0L(2,2),
				_C0L(1,2),_C0L(1,2),
				_C0L(0,1),_C0L(0,1),
				_C0L(0,1),_C0L(0,1)
			},
			//3
			{
				_C0L(3,2),_C0L(3,2),
					_C0L(2,2),_C0L(2,2),
					_C0L(1,2),_C0L(1,2),
					_C0L(0,2),_C0L(0,2)
			},
			//4
			{
				_C0L(4,3),_C0L(3,3),
					_C0L(2,2),_C0L(2,2),
					_C0L(1,2),_C0L(1,2),
					_C0L(0,2),_C0L(0,2)
				},
				//5
				{
					_C0L(5,3),_C0L(4,3),
						_C0L(3,3),_C0L(2,3),
						_C0L(1,2),_C0L(1,2),
						_C0L(0,2),_C0L(0,2)
				},
				//6
				{
					_C0L(1,3),_C0L(2,3),
						_C0L(4,3),_C0L(3,3),
						_C0L(6,3),_C0L(5,3),
						_C0L(0,2),_C0L(0,2)
					},
					//7
					{
						_C0L(0,0),_C0L(6,3),
							_C0L(5,3),_C0L(4,3),
							_C0L(3,3),_C0L(2,3),
							_C0L(1,3),_C0L(0,3)
					}
	};

#if 0
	static const char ExpGolombtab[128] = {
		((-1)>>1)+1+3, //0 
		(15>>1)+1+3, //1
		(13>>1)+1+3,(13>>1)+1+3,
		(11>>1)+1+3,(11>>1)+1+3,(11>>1)+1+3,(11>>1)+1+3,
		(9>>1)+1+3,(9>>1)+1+3,(9>>1)+1+3,(9>>1)+1+3,(9>>1)+1+3,(9>>1)+1+3,(9>>1)+1+3,(9>>1)+1+3,
		(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,
		(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,(7>>1)+1+3,
		(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,
		(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,
		(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,
		(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,(5>>1)+1+3,
		(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,
		(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,
		(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,
		(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,
		(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,
		(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,
		(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,
		(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,(3>>1)+1+3,
	};

	if(vlcnum>=6)
	{
#if 1
		code=H264GetBits ARGS1(3);
		if(code)
		{
			retval = 7 - code;
		}
		else
		{
			retval = find_leading_1 ARGS0() + 7;
		}
#else
		code=H264ShowBits ARGS1(11);

		if(code>=256)
		{
			H264AdvanceBits ARGS1(3);
			retval = 7-(code>>8);
		}
		else if(code>=128)
		{
			H264AdvanceBits ARGS1(4);
			retval = 7;
		}
		else
		{
			vlcnum = ExpGolombtab[code&0xFF];
			H264AdvanceBits ARGS1(vlcnum);
			retval = vlcnum+3;
		}
#endif
	}
	else
	{
		code = run_before[vlcnum][H264ShowBits ARGS1(3)];
		H264AdvanceBits ARGS1(code&0x0F);
		retval = code>>4;
	}

	return retval;

#else

	code = run_before[vlcnum][H264ShowBits ARGS1(3)];
	if(code==0)
	{
		H264AdvanceBits ARGS1(3);
		retval = find_leading_1 ARGS0() + 7;
	}
	else
	{
		H264AdvanceBits ARGS1(code&0x0F);
		retval = code>>4;
	}
	return retval;
#endif
}
