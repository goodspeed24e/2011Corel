/*!
***************************************************************************
* \file
*    mb_average.h
*
* \brief
*    Headerfile for two macrblocks/sub-macroblocks average
*
***************************************************************************
*/

#ifndef _MB_AVERGE_H_
#define _MB_AVERGE_H_

#include "global.h"

typedef void average_t(imgpel *output, int output_stride, imgpel *input1, imgpel *input2, int input_stride, int height);
typedef void weight_t(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height);
typedef void weight_b_t(imgpel *output, 
												int stride_out,
												imgpel *input1, imgpel *input2, 
												int weight1, int weight2, 
												int rounding_offset, unsigned int down_shift, int final_offset, 
												int stride_in, int height);
typedef void weight_uv_t(imgpel *input_image, int weight_u, int weight_v, int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, int stride_in, int height);
typedef void weight_b_uv_t(imgpel *output, 
													 int stride_out,
													 imgpel *input1, imgpel *input2, 
													 int weight1_u, int weight2_u, 
													 int weight1_v, int weight2_v,
													 int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, 
													 int stride_in, int height);

extern average_t average_16_c;
extern average_t average_16_mmx;
extern average_t average_8_c;
extern average_t average_8_mmx;
extern average_t average_4_c;
extern average_t average_4_mmx;
extern average_t average_2_c;
extern average_t average_2_mmx;

extern average_t *average_16;
extern average_t *average_8;
extern average_t *average_4;
extern average_t *average_2;


extern weight_t weight_16_c;
extern weight_t weight_16_mmx;
extern weight_t weight_16_sse2;
extern weight_t weight_8_c;
extern weight_t weight_8_mmx;
extern weight_t weight_8_sse;
extern weight_t weight_8_sse2;
extern weight_t weight_4_c;
extern weight_t weight_4_mmx;
extern weight_t weight_2_c;
extern weight_t weight_2_sse;

extern weight_b_t weight_16_b_c;
extern weight_b_t weight_16_b_sse2;
extern weight_b_t weight_16_b_mmx;
extern weight_b_t weight_8_b_c;
extern weight_b_t weight_8_b_sse2;
extern weight_b_t weight_8_b_mmx;
extern weight_b_t weight_4_b_c;
extern weight_b_t weight_4_b_mmx;
extern weight_b_t weight_2_b_c;
extern weight_b_t weight_2_b_sse;

extern weight_t *weight_16;
extern weight_t *weight_8;
extern weight_t *weight_4;
extern weight_t *weight_2;

extern weight_b_t *weight_16_b;
extern weight_b_t *weight_8_b;
extern weight_b_t *weight_4_b;
extern weight_b_t *weight_2_b;

/*
//merge UV code
*/
extern weight_uv_t *weight_8_uv;
extern weight_uv_t *weight_4_uv;
extern weight_uv_t *weight_2_uv;

extern weight_b_uv_t *weight_8_b_uv;
extern weight_b_uv_t *weight_4_b_uv;
extern weight_b_uv_t *weight_2_b_uv;

extern weight_uv_t weight_8_uv_c;
extern weight_uv_t weight_4_uv_c;
extern weight_uv_t weight_2_uv_c;

extern weight_b_uv_t weight_8_b_uv_c;
extern weight_b_uv_t weight_4_b_uv_c;
extern weight_b_uv_t weight_2_b_uv_c;

extern weight_uv_t weight_8_uv_sse2;
extern weight_uv_t weight_8_uv_sse;
extern weight_uv_t weight_4_uv_mmx;
extern weight_uv_t weight_2_uv_mmx;

extern weight_b_uv_t weight_8_b_uv_sse2;
extern weight_b_uv_t weight_8_b_uv_sse;
extern weight_b_uv_t weight_4_b_uv_sse2;
extern weight_b_uv_t weight_4_b_uv_sse;
extern weight_b_uv_t weight_2_b_uv_mmx;

#endif
