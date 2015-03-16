
/**
* @file clipping.h
* Platform-specifig clipping, min/max,  etc. macros
*
* - Copyright (c) 2002-2004 Videosoft, Inc.
* - Project:	Videosoft H.264 Codec
* - Module:	Decoder and Encoder
*/

#ifndef __CLIPPING_H__
#define __CLIPPING_H__
//#include "types.h"

#ifndef min
#define min(a,b) (((a)>(b)) ? (b) : (a))
#endif
#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

//#define CLIP0_255(a) (g_ClipTable0_255.entry[a]) 
//Nick: Table is too small for some cases. TODO: 2 different macros - 
//One when "small" overflow is garanteed, second - hard one below 
// Svitcov: best implementation for PC in cases, when x mostly within interval
#define CLIP0_255(x) (((x)&(-256)) ? (255& (~(int)(x) >> 31)) : (x)) 
#define CLIP0_X(val, x)   ((unsigned)(val) > (x) ? ((x)&(~(int)(val)>>31)) : (val))
#define CLIPX_X(val, x)   ( (unsigned)((val) + (x)) > (unsigned)((x) + (x)) ? ((((x)+(x))&(~(int)((val) + (x))>>31))-(x)) : (val))
// Clip without changing value if within range
#define DO_CLIP_3(min, max, val) if(val<min) val=min;else if(val>max) val=max

//#define CLIP0_255(x) ((x) & (~(int)(x) >> 31) | ( (~(int)(x) + 255) >> 31) & 255)
//#define CLIP0_255(x) (((x)>0) ? (x)<255 ? (x) : 255 : 0) 

//#define CLIP0_255(x) ( x & ((int)((~(x)) & ((x)-256))>>31))

/*
struct ClipTable0_255
{
unsigned char pad0[16];
unsigned char entry[256];
unsigned char pad255[16];
};

extern const struct ClipTable0_255 g_ClipTable0_255;
*/

//Tables for QP-related calulations
extern const unsigned char div_6[];
extern const unsigned char mod_6[];

/*
static INLINE int mode_52(int qp)
{
return qp < 0 ? qp + 52 : qp < 52 ? qp : qp - 52;
}	
*/
#endif //__CLIPPING_H__
