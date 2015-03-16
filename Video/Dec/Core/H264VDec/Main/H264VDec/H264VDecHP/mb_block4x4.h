#ifndef _MB_BLOCK4X4_H_
#define _MB_BLOCK4X4_H_

typedef void inverse_transform4x4_t(unsigned char *dest, unsigned char *pred,
																		short *src, int stride);
inverse_transform4x4_t inverse_transform4x4_c;
inverse_transform4x4_t inverse_transform4x4_sse; // for SSE optimization
inverse_transform4x4_t inverse_transform4x4_2_sse;
inverse_transform4x4_t inverse_transform4x4_2_sse2; // for SSE2 optimization, do 2 blocks together
extern inverse_transform4x4_t *inverse_transform4x4;

typedef void inverse_transform4x4_mergeuv_t(unsigned char *dest, unsigned char *pred,
																						short *src_u, short *src_v, int stride);
inverse_transform4x4_mergeuv_t inverse_transform4x4_mergeuv_sse2;
inverse_transform4x4_mergeuv_t inverse_transform4x4_mergeuv_sse;
inverse_transform4x4_mergeuv_t inverse_transform4x4_mergeuv_c;


#endif
