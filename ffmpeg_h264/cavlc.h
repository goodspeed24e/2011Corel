
#ifndef _CAVLC_H
#define _CAVLC_H

#include "common.h"
#include "h264.h"
#include "dsputil.h"
#include "h264context.h"

#define LUMA_DC_BLOCK_INDEX   25
#define CHROMA_DC_BLOCK_INDEX 26

#define CHROMA_DC_COEFF_TOKEN_VLC_BITS 8
#define COEFF_TOKEN_VLC_BITS           8
#define TOTAL_ZEROS_VLC_BITS           9
#define CHROMA_DC_TOTAL_ZEROS_VLC_BITS 3
#define RUN_VLC_BITS                   3
#define RUN7_VLC_BITS                  6


void decode_init_vlc(H264Context *h);
int get_level_prefix(GetBitContext *gb);
int decode_residual(H264Context *h, GetBitContext *gb, DCTELEM *block, int n, const uint8_t *scantable, int qp, int max_coeff);
int decode_mb_cavlc(H264Context *h);

#endif