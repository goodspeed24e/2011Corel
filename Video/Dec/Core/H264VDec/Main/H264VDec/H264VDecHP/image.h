
/*!
************************************************************************
* \file image.h
*
* \brief
*    prototypes for image.c
*
************************************************************************
*/

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "mbuffer.h"
#define LUMA_BLOCK_SIZE 16
//extern StorablePicture *dec_picture;

//void get_boundary_block(StorablePicture *p, int x_pos, int y_pos, int dx, int dy, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE]);

/*
typedef void (get_block_t)(imgpel *src, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride);
get_block_t get_block_p00_mmx;
get_block_t get_block_p01_mmx;
get_block_t get_block_p02_mmx;
get_block_t get_block_p03_mmx;
get_block_t get_block_p10_mmx;
get_block_t get_block_p11_mmx;
get_block_t get_block_p12_mmx;
get_block_t get_block_p13_mmx;
get_block_t get_block_p20_mmx;
get_block_t get_block_p21_mmx;
get_block_t get_block_p22_mmx;
get_block_t get_block_p23_mmx;
get_block_t get_block_p30_mmx;
get_block_t get_block_p31_mmx;
get_block_t get_block_p32_mmx;
get_block_t get_block_p33_mmx;

typedef void (get_block_t)(imgpel *src, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride);
get_block_t get_block_p00_c;
get_block_t get_block_p01_c;
get_block_t get_block_p02_c;
get_block_t get_block_p03_c;
get_block_t get_block_p10_c;
get_block_t get_block_p11_c;
get_block_t get_block_p12_c;
get_block_t get_block_p13_c;
get_block_t get_block_p20_c;
get_block_t get_block_p21_c;
get_block_t get_block_p22_c;
get_block_t get_block_p23_c;
get_block_t get_block_p30_c;
get_block_t get_block_p31_c;
get_block_t get_block_p32_c;
get_block_t get_block_p33_c;

#ifdef H264_C_ONLY
static get_block_t* get_block_4xh_fp[] = 
{	
get_block_p00_c, get_block_p01_c, get_block_p02_c, get_block_p03_c, 
get_block_p10_c, get_block_p11_c, get_block_p12_c, get_block_p13_c,
get_block_p20_c, get_block_p21_c, get_block_p22_c, get_block_p23_c,
get_block_p30_c, get_block_p31_c, get_block_p32_c, get_block_p33_c
};
#else
static get_block_t* get_block_4xh_fp[] = 
{	
get_block_p00_mmx, get_block_p01_mmx, get_block_p02_mmx, get_block_p03_mmx, 
get_block_p10_mmx, get_block_p11_mmx, get_block_p12_mmx, get_block_p13_mmx,
get_block_p20_mmx, get_block_p21_mmx, get_block_p22_mmx, get_block_p23_mmx,
get_block_p30_mmx, get_block_p31_mmx, get_block_p32_mmx, get_block_p33_mmx
};
#endif
*/

int  picture_order();

#ifdef _COLLECT_PIC_
unsigned __stdcall decode_picture_decode_ip(void *parameter);
unsigned __stdcall decode_picture_decode_b0(void *parameter);
unsigned __stdcall decode_picture_decode_b1(void *parameter);

unsigned __stdcall decode_thread(void *parameter);

CREL_RETURN initial_image PARGS0();
#endif

unsigned __stdcall decode_slice_0 (void *parameters);
unsigned __stdcall decode_slice_1 (void *parameters);
unsigned __stdcall decode_slice_2 (void *parameters);
unsigned __stdcall decode_slice_3 (void *parameters);
unsigned __stdcall decode_slice_4 (void *parameters);
unsigned __stdcall decode_slice_5 (void *parameters);

void exit_slice PARGS0();

int TransferData_at_SliceEnd PARGS0();
int StoreImgRowToImgPic PARGS2(int start_x, int end_x);

#endif

typedef void pad_boundary_luma_PX PARGS4(imgpel *ptr, int width, int height, int stride);
extern pad_boundary_luma_PX *pad_boundary_luma;
extern pad_boundary_luma_PX pad_boundary_luma_sse2;
extern pad_boundary_luma_PX pad_boundary_luma_sse;
extern pad_boundary_luma_PX pad_boundary_luma_c;

typedef void pad_boundary_chroma_PX PARGS4(imgpel *ptr, int width, int height, int stride);
extern pad_boundary_chroma_PX *pad_boundary_chroma;
extern pad_boundary_chroma_PX pad_boundary_chroma_sse2;
extern pad_boundary_chroma_PX pad_boundary_chroma_sse;
extern pad_boundary_chroma_PX pad_boundary_chroma_c;
