

#ifndef _HEAD_264
#define _HEAD_264

#include "avcodec.h"

static int decode_init(AVCodecContext *avctx);
static int decode_frame(AVCodecContext *avctx, 
                             void *data, int *data_size,
                             uint8_t *buf, int buf_size);

#endif
