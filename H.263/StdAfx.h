// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__268251D4_7C24_486C_B4E9_B3B8F706FE26__INCLUDED_)
#define AFX_STDAFX_H__268251D4_7C24_486C_B4E9_B3B8F706FE26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

typedef struct{
  int infile;                 // bit input 
  unsigned char rdbfr[2051];  //缓冲的大小
  unsigned char *rdptr;       //指向缓冲的指针
  unsigned char inbfr[16];    //16维的字符数组
  int incnt;                  //缓存中的比特数
  int bitcnt;                 //
  short block[12][64];        // block data 
}ldecode;//base,*ld;

//outtype
#define T_YUV      0
#define T_SIF      1
#define T_TGA      2
#define T_PPM      3
#define T_X11      4
#define T_YUV_CONC 5
#define T_BMP      6

typedef struct
{
   int val;
   int run;
   int sign;
}RunCoef;

static int convmat[8][4]=
{
  {117504, 138453, 13954, 34903}, /* no sequence_display_extension */
  {117504, 138453, 13954, 34903}, /* ITU-R Rec. 709 (1990) */
  {104597, 132201, 25675, 53279}, /* unspecified */
  {104597, 132201, 25675, 53279}, /* reserved */
  {104448, 132798, 24759, 53109}, /* FCC */
  {104597, 132201, 25675, 53279}, /* ITU-R Rec. 624-4 System B, G */
  {104597, 132201, 25675, 53279}, /* SMPTE 170M */
  {117579, 136230, 16907, 35559}  /* SMPTE 240M (1987) */
};

#define SF_SQCIF                        1  /* 001 */
#define SF_QCIF                         2  /* 010 */
#define SF_CIF                          3  /* 011 */
#define SF_4CIF                         4  /* 100 */
#define SF_16CIF                        5  /* 101 */

// Corresponds to CIF//////////////////////
#define MBC_MAX                         22
#define MBR_MAX                         18
#define NO_VEC                          999

////////////////////////////////////////////
#define MODE_INTER                      0
#define MODE_INTER_Q                    1
#define MODE_INTER4V                    2
#define MODE_INTRA                      3
#define MODE_INTRA_Q                    4

#define SEC                             31
#define PSC				                1
#define PSC_LENGTH			            17

static int bquant_tab[]={5,6,7,8};
static int bscan_tab[] ={2,4,6,8};
static int roundtab[16]={0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,2};

static int OM[5][8][8]={
{
  {4,5,5,5,5,5,5,4},
  {5,5,5,5,5,5,5,5},
  {5,5,6,6,6,6,5,5},
  {5,5,6,6,6,6,5,5},
  {5,5,6,6,6,6,5,5},
  {5,5,6,6,6,6,5,5},
  {5,5,5,5,5,5,5,5},
  {4,5,5,5,5,5,5,4},
},{
  {2,2,2,2,2,2,2,2},
  {1,1,2,2,2,2,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
},{
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,2,2,2,2,1,1},
  {2,2,2,2,2,2,2,2},
},{
  {0,0,0,0,1,1,1,2},
  {0,0,0,0,1,1,2,2},
  {0,0,0,0,1,1,2,2},
  {0,0,0,0,1,1,2,2},
  {0,0,0,0,1,1,2,2},
  {0,0,0,0,1,1,2,2},
  {0,0,0,0,1,1,2,2},
  {0,0,0,0,1,1,1,2},
},{
  {2,1,1,1,0,0,0,0},
  {2,2,1,1,0,0,0,0},
  {2,2,1,1,0,0,0,0},
  {2,2,1,1,0,0,0,0},
  {2,2,1,1,0,0,0,0},
  {2,2,1,1,0,0,0,0},
  {2,2,1,1,0,0,0,0},
  {2,1,1,1,0,0,0,0},
}};

//霍夫曼编码
typedef struct
{
  unsigned int code; //right justified 
  char len;
} VLCtable;

//DCT编码系数表1
static VLCtable DCT3Dtab0[]=
{
{4225,7}, {4209,7}, {4193,7}, {4177,7}, {193,7},  {177,7}, 
{161,7},  {4,7},    {4161,6}, {4161,6}, {4145,6}, {4145,6}, 
{4129,6}, {4129,6}, {4113,6}, {4113,6}, {145,6},  {145,6}, 
{129,6},  {129,6},  {113,6},  {113,6},  {97,6},   {97,6}, 
{18,6},   {18,6},   {3,6},    {3,6},    {81,5},   {81,5}, //5
{81,5},   {81,5},   {65,5},   {65,5},   {65,5},   {65,5}, 
{49,5},   {49,5},   {49,5},   {49,5},   {4097,4}, {4097,4}, 
{4097,4}, {4097,4}, {4097,4}, {4097,4}, {4097,4}, {4097,4}, 
{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2}, 
{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2}, //10
{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2}, 
{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2}, 
{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2}, 
{1,2},  {1,2},  {17,3}, {17,3}, {17,3}, {17,3}, 
{17,3}, {17,3}, {17,3}, {17,3}, {17,3}, {17,3},//15 
{17,3}, {17,3}, {17,3}, {17,3}, {17,3}, {17,3}, 
{33,4}, {33,4}, {33,4}, {33,4}, {33,4}, {33,4}, 
{33,4}, {33,4}, {2,4},  {2,4},  {2,4},  {2,4},
{2,4},  {2,4},  {2,4},  {2,4}   //19
};
//DCT编码系数表2
static VLCtable DCT3Dtab1[]= 
{
{9,10},   {8,10},   {4481,9}, {4481,9}, {4465,9}, {4465,9}, 
{4449,9}, {4449,9}, {4433,9}, {4433,9}, {4417,9}, {4417,9}, 
{4401,9}, {4401,9}, {4385,9}, {4385,9}, {4369,9}, {4369,9}, 
{4098,9}, {4098,9}, {353,9},  {353,9},  {337,9},  {337,9}, 
{321,9},  {321,9},  {305,9},  {305,9},  {289,9},  {289,9}, //5
{273,9},  {273,9},  {257,9},  {257,9},  {241,9},  {241,9}, 
{66,9},   {66,9},   {50,9},   {50,9},   {7,9},    {7,9}, 
{6,9},    {6,9},    {4353,8}, {4353,8}, {4353,8}, {4353,8}, 
{4337,8}, {4337,8}, {4337,8}, {4337,8}, {4321,8}, {4321,8}, 
{4321,8}, {4321,8}, {4305,8}, {4305,8}, {4305,8}, {4305,8}, //10
{4289,8}, {4289,8}, {4289,8}, {4289,8}, {4273,8}, {4273,8}, 
{4273,8}, {4273,8}, {4257,8}, {4257,8}, {4257,8}, {4257,8}, 
{4241,8}, {4241,8}, {4241,8}, {4241,8}, {225,8},  {225,8}, 
{225,8},  {225,8},  {209,8},  {209,8},  {209,8},  {209,8}, 
{34,8},   {34,8},   {34,8},   {34,8},   {19,8},   {19,8}, //15
{19,8},   {19,8},   {5,8},    {5,8},    {5,8},    {5,8} 
};
//DCT编码系数表3
static VLCtable DCT3Dtab2[]=
{
{4114,11}, {4114,11}, {4099,11}, {4099,11}, {11,11}, 
{11,11},   {10,11},   {10,11},   {4545,10}, {4545,10},
{4545,10}, {4545,10}, {4529,10}, {4529,10}, {4529,10},
{4529,10}, {4513,10}, {4513,10}, {4513,10}, {4513,10}, 
{4497,10}, {4497,10}, {4497,10}, {4497,10}, {146,10},//5
{146,10},  {146,10},  {146,10},  {130,10},  {130,10}, 
{130,10},  {130,10},  {114,10},  {114,10},  {114,10}, 
{114,10},  {98,10},   {98,10},   {98,10},   {98,10}, 
{82,10},   {82,10},   {82,10},   {82,10},   {51,10},
{51,10},   {51,10},   {51,10},   {35,10},   {35,10},//10
{35,10},   {35,10},   {20,10},   {20,10},   {20,10}, 
{20,10},   {12,11},   {12,11},   {21,11},   {21,11}, 
{369,11},  {369,11},  {385,11},  {385,11},  {4561,11},
{4561,11}, {4577,11}, {4577,11}, {4593,11}, {4593,11},
{4609,11}, {4609,11}, {22,12},   {36,12},   {67,12}, //15
{83,12},   {99,12},   {162,12},  {401,12},  {417,12},
{4625,12}, {4641,12}, {4657,12}, {4673,12}, {4689,12},
{4705,12}, {4721,12}, {4737,12}, {7167,7},  {7167,7},
{7167,7},  {7167,7},  {7167,7},  {7167,7},  {7167,7}, 
{7167,7},  {7167,7},  {7167,7},  {7167,7},  {7167,7}, //20
{7167,7},  {7167,7},  {7167,7},  {7167,7},  {7167,7},
{7167,7},  {7167,7},  {7167,7},  {7167,7},  {7167,7},
{7167,7},  {7167,7},  {7167,7},  {7167,7},  {7167,7}, 
{7167,7},  {7167,7},  {7167,7},  {7167,7},  {7167,7} 
};

#define deESCAPE                        7167
#define ESCAPE_INDEX                    102

#define PCT_INTER                       1
#define PCT_INTRA                       0
#define ON                              1
#define OFF                             0

#define PBMODE_NORMAL                   0
#define PBMODE_MVDB                     1
#define PBMODE_CBPB_MVDB                2

//解码的vlc表
static VLCtable TMNMVtab0[]= 
{
{3,4}, {61,4}, {2,3}, {2,3}, {62,3}, {62,3}, 
{1,2}, {1,2}, {1,2},  {1,2}, {63,2}, {63,2}, 
{63,2}, {63,2}
};
 
static VLCtable TMNMVtab1[]= 
{
{12,10}, {52,10}, {11,10}, {53,10}, {10,9}, {10,9}, 
{54,9},  {54,9},  {9,9},   {9,9},   {55,9}, {55,9}, 
{8,9},  {8,9},  {56,9}, {56,9}, {7,7},  {7,7}, 
{7,7},  {7,7},  {7,7},  {7,7},  {7,7},  {7,7}, 
{57,7}, {57,7}, {57,7}, {57,7}, {57,7}, {57,7}, 
{57,7}, {57,7}, {6,7},  {6,7},  {6,7},  {6,7}, 
{6,7},  {6,7},  {6,7},  {6,7},  {58,7}, {58,7}, 
{58,7}, {58,7}, {58,7}, {58,7}, {58,7}, {58,7}, 
{5,7},  {5,7},  {5,7},  {5,7},  {5,7},  {5,7}, 
{5,7},  {5,7},  {59,7}, {59,7}, {59,7}, {59,7}, 
{59,7}, {59,7}, {59,7}, {59,7}, {4,6},  {4,6}, 
{4,6}, {4,6}, {4,6}, {4,6}, {4,6}, {4,6}, 
{4,6}, {4,6}, {4,6}, {4,6}, {4,6}, {4,6}, 
{4,6}, {4,6}, {60,6},{60,6},{60,6},{60,6},
{60,6},{60,6},{60,6},{60,6},{60,6},{60,6},
{60,6},{60,6},{60,6},{60,6},{60,6},{60,6}
};

static VLCtable TMNMVtab2[]= 
{
{32,12}, {31,12}, {33,12}, {30,11}, {30,11}, {34,11}, 
{34,11}, {29,11}, {29,11}, {35,11}, {35,11}, {28,11}, 
{28,11}, {36,11}, {36,11}, {27,11}, {27,11}, {37,11}, 
{37,11}, {26,11}, {26,11}, {38,11}, {38,11}, {25,11}, 
{25,11}, {39,11}, {39,11}, {24,10}, {24,10}, {24,10}, 
{24,10}, {40,10}, {40,10}, {40,10}, {40,10}, {23,10}, 
{23,10}, {23,10}, {23,10}, {41,10}, {41,10}, {41,10}, 
{41,10}, {22,10}, {22,10}, {22,10}, {22,10}, {42,10}, 
{42,10}, {42,10}, {42,10}, {21,10}, {21,10}, {21,10}, 
{21,10}, {43,10}, {43,10}, {43,10}, {43,10}, {20,10}, 
{20,10}, {20,10}, {20,10}, {44,10}, {44,10}, {44,10}, 
{44,10}, {19,10}, {19,10}, {19,10}, {19,10}, {45,10}, 
{45,10}, {45,10}, {45,10}, {18,10}, {18,10}, {18,10}, 
{18,10}, {46,10}, {46,10}, {46,10}, {46,10}, {17,10}, 
{17,10}, {17,10}, {17,10}, {47,10}, {47,10}, {47,10}, 
{47,10}, {16,10}, {16,10}, {16,10}, {16,10}, {48,10}, 
{48,10}, {48,10}, {48,10}, {15,10}, {15,10}, {15,10}, 
{15,10}, {49,10}, {49,10}, {49,10}, {49,10}, {14,10}, 
{14,10}, {14,10}, {14,10}, {50,10}, {50,10}, {50,10}, 
{50,10}, {13,10}, {13,10}, {13,10}, {13,10}, {51,10}, 
{51,10}, {51,10}, {51,10}
};

//帧间编码色度块
static VLCtable MCBPCtab[]= 
{
{ERROR,0},
{255,9}, {52,9}, {36,9}, {20,9}, {49,9}, {35,8}, {35,8}, {19,8}, {19,8}, 
{50,8},  {50,8}, {51,7}, {51,7}, {51,7}, {51,7}, {34,7}, {34,7}, {34,7}, 
{34,7},  {18,7}, {18,7}, {18,7}, {18,7}, {33,7}, {33,7}, {33,7}, {33,7}, 
{17,7},  {17,7}, {17,7}, {17,7}, {4,6},  {4,6},  {4,6},  {4,6},  {4,6}, 
{4,6},   {4,6},  {4,6},  {48,6}, {48,6}, {48,6}, {48,6}, {48,6}, {48,6}, 
{48,6},  {48,6}, {3,5},  {3,5},  {3,5},  {3,5},  {3,5},  {3,5},  {3,5}, 
{3,5},  {3,5},  {3,5},  {3,5},  {3,5},  {3,5},  {3,5},  {3,5},  {3,5}, 
{32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, 
{32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, 
{32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, 
{32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {16,4}, {16,4}, {16,4}, {16,4}, 
{16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, 
{16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, 
{16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, 
{16,4},{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3} 
};
//帧内编码色度块
static VLCtable MCBPCtabintra[]=
{
{ERROR,0},
{20,6}, {36,6}, {52,6}, {4,4},  {4,4},  {4,4}, 
{4,4},  {19,3}, {19,3}, {19,3}, {19,3}, {19,3}, 
{19,3}, {19,3}, {19,3}, {35,3}, {35,3}, {35,3}, 
{35,3}, {35,3}, {35,3}, {35,3}, {35,3}, {51,3}, 
{51,3}, {51,3}, {51,3}, {51,3}, {51,3}, {51,3}, 
{51,3}
};
//亮度块的编码色
static VLCtable CBPYtab[48]=
{ {ERROR,0}, {ERROR,0}, {9,6}, {6,6}, {7,5}, {7,5}, {11,5}, {11,5},
  {13,5}, {13,5}, {14,5}, {14,5}, {15,4}, {15,4}, {15,4}, {15,4}, 
  {3,4}, {3,4}, {3,4}, {3,4}, {5,4},{5,4},{5,4},{5,4},
  {1,4}, {1,4}, {1,4}, {1,4}, {10,4}, {10,4}, {10,4}, {10,4},
  {2,4}, {2,4}, {2,4}, {2,4}, {12,4}, {12,4}, {12,4}, {12,4}, 
  {4,4}, {4,4}, {4,4}, {4,4}, {8,4}, {8,4}, {8,4}, {8,4} 
};



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__268251D4_7C24_486C_B4E9_B3B8F706FE26__INCLUDED_)
