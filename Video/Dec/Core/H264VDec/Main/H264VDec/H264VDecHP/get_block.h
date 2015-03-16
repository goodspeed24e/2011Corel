#ifndef _GET_BLOCK_H_
#define _GET_BLOCK_H_

#include "mbuffer.h"

//#define LUMA_BLOCK_SIZE 16
#define q_interpol(p0, p1, p2, p3, p4, p5) (p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5)
//extern StorablePicture *dec_picture;
void get_block PARGS6(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE]);

typedef void ( get_block_4xh_t)(imgpel *src, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height);
get_block_4xh_t get_block_4xh_p00_c;
get_block_4xh_t get_block_4xh_p01_c;
get_block_4xh_t get_block_4xh_p02_c;
get_block_4xh_t get_block_4xh_p03_c;
get_block_4xh_t get_block_4xh_p10_c;
get_block_4xh_t get_block_4xh_p11_c;
get_block_4xh_t get_block_4xh_p12_c;
get_block_4xh_t get_block_4xh_p13_c;
get_block_4xh_t get_block_4xh_p20_c;
get_block_4xh_t get_block_4xh_p21_c;
get_block_4xh_t get_block_4xh_p22_c;
get_block_4xh_t get_block_4xh_p23_c;
get_block_4xh_t get_block_4xh_p30_c;
get_block_4xh_t get_block_4xh_p31_c;
get_block_4xh_t get_block_4xh_p32_c;
get_block_4xh_t get_block_4xh_p33_c;

get_block_4xh_t get_block_4xh_p00_mmx;
get_block_4xh_t get_block_4xh_p01_mmx;
get_block_4xh_t get_block_4xh_p02_mmx;
get_block_4xh_t get_block_4xh_p03_mmx;
get_block_4xh_t get_block_4xh_p10_mmx;
get_block_4xh_t get_block_4xh_p11_mmx;
get_block_4xh_t get_block_4xh_p12_mmx;
get_block_4xh_t get_block_4xh_p13_mmx;
get_block_4xh_t get_block_4xh_p20_mmx;
get_block_4xh_t get_block_4xh_p21_mmx;
get_block_4xh_t get_block_4xh_p22_mmx;
get_block_4xh_t get_block_4xh_p23_mmx;
get_block_4xh_t get_block_4xh_p30_mmx;
get_block_4xh_t get_block_4xh_p31_mmx;
get_block_4xh_t get_block_4xh_p32_mmx;
get_block_4xh_t get_block_4xh_p33_mmx;

typedef void ( get_block_8xh_t)(imgpel *src, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height);
get_block_8xh_t get_block_8xh_p00_c;
get_block_8xh_t get_block_8xh_p01_c;
get_block_8xh_t get_block_8xh_p02_c;
get_block_8xh_t get_block_8xh_p03_c;
get_block_8xh_t get_block_8xh_p10_c;
get_block_8xh_t get_block_8xh_p11_c;
get_block_8xh_t get_block_8xh_p12_c;
get_block_8xh_t get_block_8xh_p13_c;
get_block_8xh_t get_block_8xh_p20_c;
get_block_8xh_t get_block_8xh_p21_c;
get_block_8xh_t get_block_8xh_p22_c;
get_block_8xh_t get_block_8xh_p23_c;
get_block_8xh_t get_block_8xh_p30_c;
get_block_8xh_t get_block_8xh_p31_c;
get_block_8xh_t get_block_8xh_p32_c;
get_block_8xh_t get_block_8xh_p33_c;

get_block_8xh_t get_block_8xh_p00_sse;
get_block_8xh_t get_block_8xh_p01_sse;
get_block_8xh_t get_block_8xh_p02_sse;
get_block_8xh_t get_block_8xh_p03_sse;
get_block_8xh_t get_block_8xh_p10_sse;
get_block_8xh_t get_block_8xh_p11_sse;
get_block_8xh_t get_block_8xh_p12_sse;
get_block_8xh_t get_block_8xh_p13_sse;
get_block_8xh_t get_block_8xh_p20_sse;
get_block_8xh_t get_block_8xh_p21_sse;
get_block_8xh_t get_block_8xh_p22_sse;
get_block_8xh_t get_block_8xh_p23_sse;
get_block_8xh_t get_block_8xh_p30_sse;
get_block_8xh_t get_block_8xh_p31_sse;
get_block_8xh_t get_block_8xh_p32_sse;
get_block_8xh_t get_block_8xh_p33_sse;

get_block_8xh_t get_block_8xh_p00_sse2;
get_block_8xh_t get_block_8xh_p01_sse2;
get_block_8xh_t get_block_8xh_p02_sse2;
get_block_8xh_t get_block_8xh_p03_sse2;
get_block_8xh_t get_block_8xh_p10_sse2;
get_block_8xh_t get_block_8xh_p11_sse2;
get_block_8xh_t get_block_8xh_p12_sse2;
get_block_8xh_t get_block_8xh_p13_sse2;
get_block_8xh_t get_block_8xh_p20_sse2;
get_block_8xh_t get_block_8xh_p21_sse2;
get_block_8xh_t get_block_8xh_p22_sse2;
get_block_8xh_t get_block_8xh_p23_sse2;
get_block_8xh_t get_block_8xh_p30_sse2;
get_block_8xh_t get_block_8xh_p31_sse2;
get_block_8xh_t get_block_8xh_p32_sse2;
get_block_8xh_t get_block_8xh_p33_sse2;

typedef void ( get_block_16xh_t)(imgpel *src, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height);
get_block_16xh_t get_block_16xh_p00_c;
get_block_16xh_t get_block_16xh_p01_c;
get_block_16xh_t get_block_16xh_p02_c;
get_block_16xh_t get_block_16xh_p03_c;
get_block_16xh_t get_block_16xh_p10_c;
get_block_16xh_t get_block_16xh_p11_c;
get_block_16xh_t get_block_16xh_p12_c;
get_block_16xh_t get_block_16xh_p13_c;
get_block_16xh_t get_block_16xh_p20_c;
get_block_16xh_t get_block_16xh_p21_c;
get_block_16xh_t get_block_16xh_p22_c;
get_block_16xh_t get_block_16xh_p23_c;
get_block_16xh_t get_block_16xh_p30_c;
get_block_16xh_t get_block_16xh_p31_c;
get_block_16xh_t get_block_16xh_p32_c;
get_block_16xh_t get_block_16xh_p33_c;

get_block_16xh_t get_block_16xh_p00_sse;
get_block_16xh_t get_block_16xh_p01_sse;
get_block_16xh_t get_block_16xh_p02_sse;
get_block_16xh_t get_block_16xh_p03_sse;
get_block_16xh_t get_block_16xh_p10_sse;
get_block_16xh_t get_block_16xh_p11_sse;
get_block_16xh_t get_block_16xh_p12_sse;
get_block_16xh_t get_block_16xh_p13_sse;
get_block_16xh_t get_block_16xh_p20_sse;
get_block_16xh_t get_block_16xh_p21_sse;
get_block_16xh_t get_block_16xh_p22_sse;
get_block_16xh_t get_block_16xh_p23_sse;
get_block_16xh_t get_block_16xh_p30_sse;
get_block_16xh_t get_block_16xh_p31_sse;
get_block_16xh_t get_block_16xh_p32_sse;
get_block_16xh_t get_block_16xh_p33_sse;

get_block_16xh_t get_block_16xh_p00_sse2;
get_block_16xh_t get_block_16xh_p01_sse2;
get_block_16xh_t get_block_16xh_p02_sse2;
get_block_16xh_t get_block_16xh_p03_sse2;
get_block_16xh_t get_block_16xh_p10_sse2;
get_block_16xh_t get_block_16xh_p11_sse2;
get_block_16xh_t get_block_16xh_p12_sse2;
get_block_16xh_t get_block_16xh_p13_sse2;
get_block_16xh_t get_block_16xh_p20_sse2;
get_block_16xh_t get_block_16xh_p21_sse2;
get_block_16xh_t get_block_16xh_p22_sse2;
get_block_16xh_t get_block_16xh_p23_sse2;
get_block_16xh_t get_block_16xh_p30_sse2;
get_block_16xh_t get_block_16xh_p31_sse2;
get_block_16xh_t get_block_16xh_p32_sse2;
get_block_16xh_t get_block_16xh_p33_sse2;

extern get_block_4xh_t* get_block_4xh_fp[16];
extern get_block_8xh_t* get_block_8xh_fp[16];
extern get_block_16xh_t* get_block_16xh_fp[16];

extern get_block_4xh_t*  get_block_4xh_c_fp[16];
extern get_block_8xh_t*  get_block_8xh_c_fp[16];
extern get_block_16xh_t* get_block_16xh_c_fp[16];

extern get_block_4xh_t*  get_block_4xh_mmx_fp[16];
extern get_block_8xh_t*  get_block_8xh_sse2_fp[16];
extern get_block_16xh_t* get_block_16xh_sse2_fp[16];

extern get_block_8xh_t*  get_block_8xh_sse_fp[16];
extern get_block_16xh_t* get_block_16xh_sse_fp[16];

typedef void ( get_block_wxh) PARGS7(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int height);
get_block_wxh get_block_4xh_full;
get_block_wxh get_block_8xh_full;
get_block_wxh get_block_16xh_full;

get_block_wxh get_block_4xh_int;
get_block_wxh get_block_8xh_int;
get_block_wxh get_block_16xh_int;

#endif
