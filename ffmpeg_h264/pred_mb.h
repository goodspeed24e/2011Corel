
#ifndef _PRED_MB_H
#define _PRED_MB_H

#include "h264.h"
#include "h264context.h"

/**
 * gets the predicted number of non zero coefficients.
 * @param n block index
 */

int pred_intra_mode(H264Context *h, int n);
void write_back_non_zero_count(H264Context *h);
int pred_non_zero_count(H264Context *h, int n);

int fetch_diagonal_mv(H264Context *h, const int16_t **C, int i, int list, int part_width);

void pred_motion(H264Context * const h, int n, int part_width, int list, int ref, int * const mx, int * const my);
void pred_16x8_motion(H264Context * const h, int n, int list, int ref, int * const mx, int * const my);
void pred_8x16_motion(H264Context * const h, int n, int list, int ref, int * const mx, int * const my);
void pred_pskip_motion(H264Context * const h, int * const mx, int * const my);

void write_back_intra_pred_mode(H264Context *h);
int check_intra4x4_pred_mode(H264Context *h);
int check_intra_pred_mode(H264Context *h, int mode);
void write_back_motion(H264Context *h, int mb_type);

void init_pred_ptrs(H264Context *h);

#endif
