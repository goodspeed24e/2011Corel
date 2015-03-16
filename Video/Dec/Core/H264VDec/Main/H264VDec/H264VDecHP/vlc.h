
/*!
************************************************************************
* \file vlc.h
*
* \brief
*    header for (CA)VLC coding functions
*
* \author
*    Karsten Suehring
*
************************************************************************
*/

#ifndef _VLC_H_
#define _VLC_H_

extern const unsigned char _NCBP[2][48][2];

#define ue_v(string)           ue_v_no_string  ARGS0()
#define se_v(string)           se_v_no_string  ARGS0()
#define u_v(LenInBits, string) u_v_no_string ARGS1(LenInBits)
#define u_1(string)            u_1_no_string ARGS0()

int readSyntaxElement_Intra4x4PredictionMode PARGS0();
int read_raw_mb_uvlc PARGS0();
int readMVD_uvlc PARGS2(int list, MotionVector *mvd);

int ue_v_no_string PARGS0();
int se_v_no_string PARGS0();
int u_v_no_string PARGS1(int LenInBits);
int u_1_no_string PARGS0();

// UVLC mapping
int readSyntaxElement_VLC_ue PARGS0();
int readSyntaxElement_VLC_se PARGS0();
int readSyntaxElement_VLC_cbp_intra PARGS0();
int readSyntaxElement_VLC_cbp_inter PARGS0();
int readSyntaxElement_FLC PARGS1(int nbits);
int peekSyntaxElement_FLC PARGS1(int nbits);

int readSyntaxElement_NumCoeffTrailingOnes PARGS1(int vlcnum);
int readSyntaxElement_NumCoeffTrailingOnesChromaDC PARGS0();
int readSyntaxElement_Level_VLC PARGS1(int vlcnum);
int readSyntaxElement_TotalZeros PARGS1(int vlcnum);
int readSyntaxElement_TotalZerosChromaDC PARGS1(int vlcnum);
int readSyntaxElement_Run PARGS1(int vlcnum);

int more_rbsp_data PARGS0();
// added by Lifeng
int read_raw_cbp_uvlc PARGS0();
int read_raw_dquant_uvlc PARGS0();
int read_transform_size_uvlc PARGS0();

#endif

