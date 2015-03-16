
#ifndef _FILL_FUNC_H
#define _FILL_FUNC_H

#include "h264.h"
#include "h264context.h"

void fill_caches(H264Context *h, int mb_type);
void fill_rectangle(void *vp, int w, int h, int stride, uint32_t val, int size);


#endif
