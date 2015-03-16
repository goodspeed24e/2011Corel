#ifndef _FILTERMB_H
#define _FILTERMB_H

//#include "portab.h"
#include "h264.h"
int get_chroma_qp(H264Context *h, int qscale);

void filter_mb(H264Context *h,
			   int mb_x,
			   int mb_y, 
			   uint8_t *img_y,
			   uint8_t *img_cb,
			   uint8_t *img_cr);

//static void filter_mb( H264Context *h, int mb_x, int mb_y, uint8_t *img_y, uint8_t *img_cb, uint8_t *img_cr);


#endif

